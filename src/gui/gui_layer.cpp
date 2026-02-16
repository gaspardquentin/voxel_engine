#include "voxel_engine/gui_layer.h"
#include "voxel_engine/voxel_engine.h"

#include <imgui.h>
#include <iostream>

void voxeng::GUILayer::drawDebugUI(const VoxelEngine& engine, float fps) {
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
    if (ImGui::Begin("Engine Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        Vec3f player_pos = engine.getCamera().getPos();
        ImGui::Text("FPS: %d", (int) fps);
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", player_pos.x, player_pos.y, player_pos.z);
    }
    ImGui::End();
}



void voxeng::GUILayer::drawSavesUI(VoxelEngine& engine) {
    ImGui::SetNextWindowPos(ImVec2(10, 100), ImGuiCond_Appearing);
    if (ImGui::Begin("Save Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::Button("Save World")) {
            engine.getSaveManager().saveWorld(engine.getWorld());
        }

        ImGui::Separator();
        ImGui::Text("Load Save:");

        SaveID last_save = engine.getSaveManager().getLastSave();
        for (SaveID i = 0; i < last_save; ++i) {
            std::string label = "Load Save " + std::to_string(i);
            if (ImGui::Button(label.c_str())) {
                World loaded_world = engine.getSaveManager().loadWorld(i);
                if (loaded_world.getSeed() != 0) {
                     engine.setWorld(std::move(loaded_world));
                } else {
                    std::cerr << "Failed to load save " << i << "\n";
                }
            }
        }
    }
    ImGui::End();
}
