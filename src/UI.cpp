#include <imgui.h>

#include "UI.hpp"


namespace UI {

    ImVec4
    get_key_color()
    {
        return {1.0f, 1.0f, 0.5f, 1.0f};
    }


    void
    key_label(const std::string& text,
              bool align_to_frame_padding)
    {
        if (align_to_frame_padding)
            ImGui::AlignTextToFramePadding();
        ImGui::TextColored(get_key_color(), "%s", text.data());
    }


    void
    flow_radio_button(const std::string& label,
                      bool value,
                      float width)
    {
        auto& style = ImGui::GetStyle();

        ImGuiChildFlags child_flags = ImGuiChildFlags_AutoResizeY;

        ImVec2 padding{};
        if (child_flags & ImGuiChildFlags_FrameStyle)
            padding = style.FramePadding;
        if (child_flags & (ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding))
            padding = style.WindowPadding;

        // float border = 0;
        // if (child_flags & ImGuiChildFlags_FrameStyle)
        //     border = style.FrameBorderSize;
        // else if (child_flags & ImGuiChildFlags_Borders)
        //     border = style.ChildBorderSize;

        auto label_size = ImGui::CalcTextSize(label.data(), nullptr, true);
        float checkbox_size = ImGui::GetFrameHeight();

        float content_width =
            label_size.x
            + checkbox_size
            + style.ItemSpacing.x
            + 2 * padding.x;
        // + 2 * border;
        if (content_width < width)
            content_width = width;

        auto available = ImGui::GetContentRegionAvail();
        float cur_x = ImGui::GetCursorScreenPos().x;
        float max_x = cur_x + available.x;

        if (ImGui::BeginChild(label.data(), {content_width, 0}, child_flags)) {
            auto available = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPosX(available.x - label_size.x - checkbox_size - style.ItemSpacing.x);
            key_label(label, true);
            ImGui::SameLine();
            ImGui::SetCursorPosX(available.x - checkbox_size);
            ImGui::RadioButton("##radio_button", value);
        }
        ImGui::EndChild();
        float spacing = style.ItemSpacing.x;
        float new_x = ImGui::GetItemRectMax().x;
        // Assume next item has same width.
        float next_width = ImGui::GetItemRectSize().x;
        // if another item will fit, keep the same line
        if (new_x + spacing + next_width < max_x)
            ImGui::SameLine();
    }

} // namespce UI
