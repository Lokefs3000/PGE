#include <Core/Application.hpp>

#include "EditorLayer.hpp"

#include "Standalone/ShaderCompiler.hpp"

#include "Utility/FileUtility.hpp"
#include "Resources/Shaders/ShaderParser.hpp"

int main(int argc, char* argv[])
{
    if (argc > 2)
    {
        if (strcmp(argv[1], "shader") == 0)
        {
            ed::ShaderCompiler sc{};
            sc.CompileArgs(argv, argc);
            return 0;
        }
    }

    std::unique_ptr<pge::Application> app =
        std::make_unique<pge::Application>();

    app->PushLayer<ed::EditorLayer>();
    app->Run();
}
