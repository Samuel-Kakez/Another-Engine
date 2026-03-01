#include "Core/Engine.h"

// Inclusions nécessaires pour l'initialisation et la boucle de jeu
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Inclusion de nos systèmes
#include "Managers/ResourceManager.h"
#include "Managers/LightManager.h"
#include "Managers/InputManager.h"
#include "Renderer/Renderer.h"
#include "Core/Scene.h"
#include "Utils/SceneSerializer.h"
#include "Core/EventDispatcher.h"
#include "Debug/StatsManager.h"
#include "Debug/Logger.h"

// --- Déclaration de la fonction de callback ---
// Elle est maintenant déclarée ici, car seul l'Engine en a besoin.
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

Engine::Engine() : m_window(nullptr)
{
    // Phase 1 : Initialisation
    if (!glfwInit())
    {
        LOG_ERROR("Échec de l'initialisation de GLFW.");
        return;
    }
    m_glfwInitialized = true;
    LOG_INFO("GLFW initialisé (OpenGL4.3 Core).");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    m_window = glfwCreateWindow(mode->width, mode->height, "Another Engine", monitor, nullptr);

    if (m_window == NULL)
    {
        LOG_ERROR("Échec de la création de la fenêtre GLFW.");
        glfwTerminate();
        m_glfwInitialized = false;
        return;
    }

    m_windowCreated = true;
    LOG_INFO("fenêtre créée (%dx%d).", mode->width, mode->height);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // V-Sync
    // On stocke un pointeur vers cette instance de l'Engine dans la fenêtre
    // Cela nous permettra de le récupérer dans le callback
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERROR("Échec de l'initialisation de GLAD.");
        if (m_windowCreated && m_window)
        {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
            m_windowCreated = false;
        }
        if (m_glfwInitialized)
        {
            glfwTerminate();
            m_glfwInitialized = false;
        }
        return;
    }
    m_glContextReady = true;
    LOG_INFO("GLAD initialisé.");

    if (GLAD_GL_VERSION_4_3)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(
            [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
            {
                // Ignorer les messages "notification", trop verbeux
                if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
                {
                    return;
                }

                if (severity == GL_DEBUG_SEVERITY_HIGH)
                {
                    LOG_ERROR("[OpenGL] %s", message);
                }
                else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
                {
                    LOG_WARN("[OpenGL] %s", message);
                }
                else
                {
                    LOG_TRACE("[OpenGL] %s", message);
                }
            }, nullptr);
            LOG_INFO("debug callback OpenGL activé (GL 4.3 core).");
    }
    else{
        LOG_WARN("GL 4.3 non disponible - pas de debug callback OpenGL.");
    }

    // On récupère la taille réelle du framebuffer
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

    // Et on configure le viewport d'OpenGL pour qu'il utilise toute la surface
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    LOG_INFO("viewport configuré (%dx%d).", framebufferWidth, framebufferHeight);

    // 1. On Crée le bus de communication en premier
    auto eventDispatcher = std::make_unique<EventDispatcher>();
    LOG_INFO("EventDispatcher créé.");

    // 2. On créé les managers et systèmes autonomes
    m_resourceManager = std::make_unique<ResourceManager>();
    LOG_INFO("sous-système ResourceManager créé.");
    m_lightManager = std::make_unique<LightManager>(*eventDispatcher);
    LOG_INFO("sous-système LightManager créé.");
    m_inputManager = std::make_unique<InputManager>(m_window);
    LOG_INFO("sous-système InputManager créé.");

    // 3. On créé les systèmes qui doivent écouter les événements, en leur passant le dispatcher.
    m_renderer = std::make_unique<Renderer>(*m_resourceManager, *eventDispatcher, m_window);
    LOG_INFO("sous-système Renderer créé.");
    m_debugUI = std::make_unique<DebugUI>(m_window);
    LOG_INFO("sous-système DebugUI créé.");

    // 4. On créé la scène en dernier, en lui injectant ses dépendances.
    //  Elle prend possession du dispatcher via std::move
    m_scene = std::make_unique<Scene>(
        std::move(eventDispatcher),
        *m_resourceManager,
        *m_lightManager,
        *m_inputManager,
        *m_renderer);
    LOG_INFO("Scène créée - initialisation terminée.");
    m_initialized = true;
}

Engine::~Engine()
{
    LOG_INFO("arrêt en cours, nettoyage des ressources...");

    // =======================================
    // Ordre de destruction critique
    // =======================================

    // 1. DebugUI - pas de dépendance sur les autres systèmes
    m_debugUI.reset();
    // 2. Renderer - se désabonne du dispatcher (possédé par Scene)
    m_renderer.reset();
    // 3. LightManager - se désabonne du dispatcher (possédé par Scene)
    m_lightManager.reset();
    // 4. Scene - possède le dispatcher + gameObjects
    m_scene.reset();

    // 5. ResourceManager - libère ressources GPU
    if (m_resourceManager)
    {
        m_resourceManager->Clear();
        m_resourceManager.reset();
    }

    // 6. InputManager - pas de dépendance
    m_inputManager.reset();

    if (m_windowCreated && m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        m_windowCreated = false;
    }

    if (m_glfwInitialized)
    {
        glfwTerminate();
        m_glfwInitialized = false;
    }
}

void Engine::Run()
{
    // Vérifie si le constructeur est initialisé
    if (!m_initialized)
    {
        return;
    }

    // --- Prêt pour le chargement et la boucle ---

    // On utilise notre sérialiseur pour peupler la scène
    // Le ResourceManager se charge de générer les meshes pas encore existantes

    SceneSerializer serializer(*m_scene);
    serializer.Deserialize("main.scene.json");
    LOG_INFO("boucle de jeu démarrée.");

    // Boucle de jeu à pas de temps fixe
    // Le pas de temps fixe pour la logique (ici 60 mises à jour par seconde)
    const float fixedDeltaTime = 1.0f / 60.0f;

    // l'accumulateur stocke le temps réel écoulé qui n'a pas encore été simulé
    double accumulator = 0.0;

    // on garde une référence au temps du dernier tick de la boucle
    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(m_window))
    {
        double currentTime = glfwGetTime();
        // frameTime est le temps réel écoulé depuis la dernière image. Peut varier énormément
        double frameTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // On ajoute le temps réel écoulé à notre accumulateur
        accumulator += frameTime;

        StatsManager::Reset();

        // On gère les événements
        glfwPollEvents();
        m_inputManager->Update();

        // Tant que l'accumulateur contient assez de temps pour au moins un pas de logique, on exécute la logique
        while (accumulator >= fixedDeltaTime)
        {
            // logique de jeu et physique
            m_scene->FixedUpdate(fixedDeltaTime);
            // on retire le pas de temps qu'on vient de simuler
            accumulator -= fixedDeltaTime;
        }

        m_scene->FlushPendingInitialization();
        // logique non-critique qui peut dépendre du framerate
        m_scene->Update(static_cast<float>(frameTime));
        m_scene->FlushPendingInitialization();

        if (m_resizePending)
        {
            glViewport(0, 0, m_pendingWidth, m_pendingHeight);
            m_resizePending = false;
        }

        // Rendu (une seule fois par frame, peu importe le nombre de FixedUpdate)
        m_renderer->Render(*m_scene, m_window);

        // Rendu de l'UI
        m_debugUI->NewFrame();
        m_debugUI->DrawStatsWindow(static_cast<float>(frameTime));
        m_debugUI->DrawLogWindow();
        m_debugUI->Render();

        // nettoyage de fin de frame
        m_scene->ProcessDestruction();

        // échange des buffers d'affichage
        glfwSwapBuffers(m_window);
    }
}

void Engine::RequestResize(int width, int height)
{
    m_resizePending = true;
    m_pendingWidth = width;
    m_pendingHeight = height;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // On force le rendu d'une frame lors du resize
    Engine *engine = static_cast<Engine *>(glfwGetWindowUserPointer(window));
    if (engine)
    {
        engine->RequestResize(width, height);
    }
}

void Engine::RenderOneFrame()
{
    // Skip si pas initialisé
    if (!m_scene || !m_renderer || !m_window)
    {
        return;
    }
    m_renderer->Render(*m_scene, m_window);

    m_debugUI->NewFrame();
    m_debugUI->DrawStatsWindow(0.016f);
    m_debugUI->DrawLogWindow();
    m_debugUI->Render();

    glfwSwapBuffers(m_window);
}