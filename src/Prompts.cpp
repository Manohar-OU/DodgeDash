#include "Prompts.h"

Prompts::~Prompts()
{
}

void Prompts::getPrompts()
{
    static Aftr::AftrImGui_Markdown_Renderer md_render = Aftr::make_default_MarkdownRenderer();

    ImGui::Begin("Markdown");
    Aftr::AftrImGui_Markdown_Doc doc{ std::filesystem::path{ "../Prompts.md" } };
    md_render.draw_markdown(doc);
    ImGui::End();
}
