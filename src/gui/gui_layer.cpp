#include "voxel_engine/gui_layer.h"
#include "voxel_engine/voxel_engine.h"
#include "voxel_engine/save_manager.h"

#include <imgui.h>
#include <iostream>
#include <filesystem>

void voxeng::GUILayer::drawDebugUI(const VoxelEngine& engine, float fps) {
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
    if (ImGui::Begin("Engine Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        Vec3f player_pos = engine.getCamera().getPos();
        ImGui::Text("FPS: %d", (int) fps);
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", player_pos.x, player_pos.y, player_pos.z);
    }
    ImGui::End();
}

void voxeng::GUILayer::drawWorldsUI(VoxelEngine& engine) {
    ImGui::SetNextWindowPos(ImVec2(10, 200), ImGuiCond_Appearing);
    if (ImGui::Begin("World Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

        // New World section
        ImGui::Text("New World:");
        ImGui::InputText("Name", m_world_name_buf, sizeof(m_world_name_buf));
        if (ImGui::Button("Create World")) {
            std::string name(m_world_name_buf);
            if (!name.empty()) {
                engine.createNewWorld(name);
            }
        }

        ImGui::Separator();

        // Existing worlds
        ImGui::Text("Existing Worlds:");
        auto worlds = voxeng::SaveManager::listWorlds();
        for (const auto& world_path : worlds) {
            std::string dir_name = std::filesystem::path(world_path).filename().string();
            std::string label = "Load: " + dir_name;
            if (ImGui::Button(label.c_str())) {
                engine.loadExistingWorld(world_path);
            }
        }

        if (worlds.empty()) {
            ImGui::TextDisabled("No worlds found");
        }

        ImGui::Separator();

        // Save all button
        if (engine.getSaveManager().isWorldOpen()) {
            if (ImGui::Button("Save All Chunks")) {
                engine.getWorld().flushAllDirtyChunks();
            }
        }
    }
    ImGui::End();
}

void voxeng::GUILayer::drawChatUI(VoxelEngine& engine, std::string username) {
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
}
