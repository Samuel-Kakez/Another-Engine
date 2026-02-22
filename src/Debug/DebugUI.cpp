#include "Debug/DebugUI.h"
#include "Debug/StatsManager.h"
#include "Debug/Logger.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

DebugUI::DebugUI(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    // Initialisation pour GLFW et OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    m_window = window;
}

DebugUI::~DebugUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DebugUI::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::DrawStatsWindow(float deltaTime)
{
    // -- Lissage du frametime --

    if (m_isFirstFrame)
    {
        // Au premier passage, on init simplement la valeur lissée
        m_smoothedDeltaTime = deltaTime;
        m_isFirstFrame = false;
    }
    else
    {
        // On lisse la valeur en la "tirant" légèrement vers la nouvelle valeur
        // Un facteur plus petit = lissage plus fort
        const float smoothingFactor = 0.05f;
        m_smoothedDeltaTime = (deltaTime * smoothingFactor) + (m_smoothedDeltaTime * (1.0f - smoothingFactor));
    }

    // -- Affichage imGui

    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
    ImGui::Begin("Statistiques de Rendu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    // Calcul des valeurs en temps réel
    float realTimeFps = 1.0f / deltaTime;
    float realTimeMs = deltaTime * 1000.0f;

    // Calcul des valeurs lissées

    float smoothedFps = 1.0f / m_smoothedDeltaTime;
    float smoothedMs = m_smoothedDeltaTime * 1000.0f;

    ImGui::Text("Frametime : %.2f ms (%.2f ms avg)", realTimeMs, smoothedMs);
    ImGui::Text("FPS : %.1f (%.1f avg)", realTimeFps, smoothedFps);
    ImGui::Separator();
    ImGui::Text("Appels de dessin: %d", StatsManager::drawCalls);
    ImGui::Text("Triangles: %d", StatsManager::triangles);
    ImGui::Text("Lots de materiaux: %d", StatsManager::materialBinds);
    ImGui::End();
}

void DebugUI::DrawLogWindow()
{
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    float windowWidth = static_cast<float>(fbWidth);
    float windowHeight = static_cast<float>(fbHeight);

    ImGui::SetNextWindowPos(ImVec2(0.0f, windowHeight - m_logWindowHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, m_logWindowHeight), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

    ImGui::Begin("Console de Logs", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    // Boutons de filtrage par niveau
    auto ToggleBtn = [](const char *label, bool &active, const ImVec4 &col)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, active ? col : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        if (ImGui::SmallButton(label))
            active = !active;
        ImGui::PopStyleColor();
        ImGui::SameLine();
    };

    ToggleBtn("Trace", m_showTrace, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ToggleBtn("Info", m_showInfo, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ToggleBtn("Warn", m_showWarn, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
    ToggleBtn("Error", m_showError, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    m_logFilter.Draw("##filtre", -1.0f);
    ImGui::Separator();

    // -- Zone scrollable --
    ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    const auto &entries = Logger::GetEntries();
    for (const auto &entry : entries)
    {
        if (entry.level == LogLevel::Trace && !m_showTrace)
            continue;
        if (entry.level == LogLevel::Info && !m_showInfo)
            continue;
        if (entry.level == LogLevel::Warning && !m_showWarn)
            continue;
        if (entry.level == LogLevel::Error && !m_showError)
            continue;

        // Construction de la ligne affichée
        const char *levelTag = "";
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

        switch (entry.level)
        {
        case LogLevel::Trace:
            levelTag = "[TRACE]";
            color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
            break;
        case LogLevel::Info:
            levelTag = "[INFO]";
            color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        case LogLevel::Warning:
            levelTag = "[WARN]";
            color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
            break;
        case LogLevel::Error:
            levelTag = "[ERROR]";
            color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            break;
        }

        // Formatage final : "[LEVEL] (0.00s) message"
        char lineBuf[1280];
        snprintf(lineBuf, sizeof(lineBuf), "%s (%.2fs) [%s] %s", levelTag, entry.timestamp, entry.source.c_str(), entry.message.c_str());

        // Filtrage par texte
        if (!m_logFilter.PassFilter(lineBuf))
            continue;

        ImGui::TextColored(ImVec4(0.3f, 0.5f, 1.0f, 1.0f), "[Another Engine]");
        ImGui::SameLine(0, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(lineBuf);
        ImGui::PopStyleColor();
    }

    // Auto-scroll vers le bas
    if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleColor(3);
}

void DebugUI::Render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}