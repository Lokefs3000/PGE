// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include "memory"
#include "vector"
#include "functional"
#include "initguid.h"
#include "map"
#include "atomic"
#include "filesystem"

#include "DirectXMath.h"

#include "Events/Signal.hpp"

#include "entt/entt.hpp"
#include "taskflow/taskflow.hpp"
#include "Volk/volk.h"
#include "toml++/toml.hpp"
#include "vma/vk_mem_alloc.h"
#include "d3d12.h"
#include "dxgi1_6.h"
#include "D3D12MemAlloc.h"

#endif //PCH_H
