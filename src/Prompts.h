#pragma once

#include "WOImGuiAbstract.h"
#include "AftrImGui_Markdown_Renderer.h"
#include <WOImGui.h>


class Prompts : public Aftr::WOImGuiAbstract
{
public:
    virtual ~Prompts();

    static void getPrompts();
};
