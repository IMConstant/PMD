#include <imgui.h>
#include "Application.h"

#include "GUI.h"


namespace GUI {

    bool showFileSystemWindow = false;
    std::string ParseData::lastDirectory = fs::getCurrentDirectory();

    void createFileSystemWindow() {
        ImGui::SetNextWindowSize(ImVec2{400, 300}, ImGuiCond_Once);

        if (showFileSystemWindow) {
            ImGui::Begin("Select file", &showFileSystemWindow);

            fs::FilesList files = fs::parseDirectory(ParseData::lastDirectory);

            std::vector<std::string> path = string_split(ParseData::lastDirectory, "/", 0, false);

            for (uint i = 0; i < path.size(); i++) {
                std::string const &path_dir = path.at(i);
                ImGui::SameLine();
                ImGui::Text("/");
                ImGui::SameLine();

                if (ImGui::Button(path_dir.c_str())) {
                    ParseData::lastDirectory = "";

                    for (uint j = 0; j <= i; j++) {
                        ParseData::lastDirectory += "/" + path.at(j);
                    }

                    std::string pp = ParseData::lastDirectory;

                    break;
                }
            }

            ImGui::BeginChild("ChildL");
            {

                for (auto const &file : files) {
                    std::string type_str;

                    switch (file.first) {
                        case fs::FileType::Directory:
                            type_str = "[DIR] ";
                            break;
                        case fs::FileType::Regular:
                            type_str = "[FILE] ";
                            break;
                    }

                    ImGui::TextColored(
                            file.first == fs::FileType::Directory ? ImVec4(1.f, 1.f, 0.f, 1.f) : ImVec4(1.f, 1.f, 1.f,
                                                                                                        1.f),
                            type_str.c_str());
                    ImGui::SameLine();

                    if (ImGui::Selectable(file.second.c_str())) {
                        if (file.first == fs::FileType::Directory) {
                            if (file.second == "..") {
                                std::string &ld = ParseData::lastDirectory;
                                ld.erase(ld.begin() + ld.find_last_of('/'), ld.end());
                            } else {
                                ParseData::lastDirectory += std::string("/") + file.second;
                            }
                        }
                        else if (file.first == fs::FileType::Regular) {
                            Application::getInstance()->reloadMesh(ParseData::lastDirectory + std::string("/") + file.second);
                            showFileSystemWindow = false;
                        }

                        break;
                    }
                }

                ImGui::EndChild();
            }

            ImGui::End();
        } else {
            ParseData::lastDirectory = fs::getCurrentDirectory();
        }
    }

}// namespace GUI