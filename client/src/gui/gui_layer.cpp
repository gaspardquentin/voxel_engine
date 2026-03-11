#include "voxel_engine/client/gui_layer.h"

#include <imgui.h>
#include <filesystem>

namespace voxeng::client {

void voxeng::client::GUILayer::drawDebugUI(Client& client, float fps) {
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
    if (ImGui::Begin("Engine Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        Vec3f player_pos = client.getCamera().getPos();
        ImGui::Text("FPS: %d", (int) fps);
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", player_pos.x, player_pos.y, player_pos.z);

        ImGui::Separator();
        ImGui::Text("GUI Scale");
        if (m_gui_scale == 0.0f)
            m_gui_scale = ImGui::GetIO().FontGlobalScale;
        ImGui::SliderFloat("##gui_scale", &m_gui_scale, 0.5f, 3.0f, "%.1fx");
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            float ratio = m_gui_scale / ImGui::GetIO().FontGlobalScale;
            ImGui::GetStyle().ScaleAllSizes(ratio);
            ImGui::GetIO().FontGlobalScale = m_gui_scale;
        }

        if (client.isWorldOpen()) {
            ImGui::Separator();
            if (ImGui::Button("Save & Exit World")) {
                client.saveWorld();
                client.leaveWorld();
            }
        }
    }
    ImGui::End();
}

void GUILayer::drawStartMenuUI(Client& client) {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Start Menu", nullptr, flags)) {

        // New World section
        ImGui::Text("New World:");
        ImGui::InputText("Name", m_world_name_buf, sizeof(m_world_name_buf));
        if (ImGui::Button("Create World")) {
            std::string name(m_world_name_buf);
            if (!name.empty()) {
                client.createWorld(name);
            }
        }

        ImGui::Separator();

        // Existing worlds
        ImGui::Text("Existing Worlds:");
        auto worlds = client.listWorlds();
        for (const auto& world_path : worlds) {
            std::string dir_name = std::filesystem::path(world_path).filename().string();
            std::string label = "Load: " + dir_name;
            if (ImGui::Button(label.c_str())) {
                client.loadWorld(world_path);
            }
        }

        if (worlds.empty()) {
            ImGui::TextDisabled("No worlds found");
        }

        ImGui::Separator();

        // Save all button
        if (client.isWorldOpen()) {
            if (ImGui::Button("Save All Chunks")) {
                client.saveWorld();
            }
        }
    }
    ImGui::End();
}

void voxeng::client::GUILayer::drawChatUI(Client& client, std::string username) {
    /* TODO: make it work on client/server arch
    ImGui::SetNextWindowPos(ImVec2(10, 1500), ImGuiCond_Appearing);
    if (ImGui::Begin("Chat", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        for (const Message& msg : engine.getChat().getAllMessages()) {
            ImGui::Text("[%s][%s] %s", msg.sender_id.c_str(), msg.getFormattedTime().c_str(), msg.content.c_str());
        }

        bool send = ImGui::InputTextWithHint("##chat_input", "Type a message or /command...",
            m_chat_input_buf, sizeof(m_chat_input_buf), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        send |= ImGui::Button("Send");

        if (send && m_chat_input_buf[0] != '\0') {
            engine.handleChatInput(username, std::string(m_chat_input_buf));
            m_chat_input_buf[0] = '\0';
        }
    }
    ImGui::End();
    */
}

}
