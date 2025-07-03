#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <dwmapi.h>
#include <d3d9.h>
#include "../game/actorloop.h"
#include "../settings/settings.h"
#include "../external/gui.h"
#include "../offsets SDK/sdk.h"
#include "../kernel/coms.h"
#include "../Offsets SDK/offsets.h"
#include <chrono>
#pragma comment(lib, "d3d9.lib")
#include <dwmapi.h>
#include <windows.h>
#include <psapi.h>
#include <string>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dwmapi.lib")
IDirect3D9Ex* p_object = NULL;
IDirect3DDevice9Ex* p_device = NULL;
D3DPRESENT_PARAMETERS p_params = { NULL };
MSG messager = { NULL };
HWND MyWnd = NULL;
DWORD ProcId;

// Aimbot-Settings
namespace settings {
	namespace aimbot {
		inline bool enable = false;
		inline float fov = 50.0f;
		inline float smooth = 5.0f;
		inline int bone = 0; // 0=Kopf, 1=Brust, 2=Bauch
		inline bool show_fov = false; // FOV-Kreis anzeigen
	}
}

const char* aimbot_bone_names[] = { "Kopf", "Brust", "Bauch" };
const int aimbot_bone_ids[] = { 110, 66, 2 };

void move_mouse(float x, float y) {
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.dx = static_cast<LONG>(x);
	input.mi.dy = static_cast<LONG>(y);
	SendInput(1, &input, sizeof(INPUT));
}

void run_aimbot() {
	if (!settings::aimbot::enable) return;
	float bestDist = settings::aimbot::fov;
	Vector2 bestDelta = { 0, 0 };
	bool found = false;
	POINT screen;
	GetCursorPos(&screen);
	float screen_center_x = width / 2.0f;
	float screen_center_y = height / 2.0f;
	int bone_id = aimbot_bone_ids[settings::aimbot::bone];
	for (const auto& player : EntityListA) {
		if (player.pawn_private == cache::local_pawn) continue;
		Vector3 bone3d = get_entity_bone(player.Mesh, bone_id);
		Vector2 bone2d = project_world_to_screen(Vector3(bone3d.x, bone3d.y, bone3d.z + 15));
		float dx = bone2d.x - screen_center_x;
		float dy = bone2d.y - screen_center_y;
		float dist = sqrtf(dx * dx + dy * dy);
		if (dist < bestDist) {
			bestDist = dist;
			bestDelta = { dx, dy };
			found = true;
		}
	}
	if (found && (GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
		float smooth = settings::aimbot::smooth;
		move_mouse(bestDelta.x / smooth, bestDelta.y / smooth);
	}
}

int getFps()
{
	using namespace std::chrono;
	static int count = 0;
	static auto last = high_resolution_clock::now();
	auto now = high_resolution_clock::now();
	static int fps = 0;
	count++;
	if (duration_cast<milliseconds>(now - last).count() > 1000)
	{
		fps = count;
		count = 0;
		last = now;
	}
	return fps;
}
HRESULT directx_init()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_object))) exit(3);
	ZeroMemory(&p_params, sizeof(p_params));
	p_params.Windowed = TRUE;
	p_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_params.hDeviceWindow = MyWnd;
	p_params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_params.BackBufferWidth = width;
	p_params.BackBufferHeight = height;
	p_params.EnableAutoDepthStencil = TRUE;
	p_params.AutoDepthStencilFormat = D3DFMT_D16;
	p_params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	if (FAILED(p_object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, MyWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_params, 0, &p_device)))
	{
		p_object->Release();
		exit(4);
	}
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(MyWnd);
	ImGui_ImplDX9_Init(p_device);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImFontAtlas* fontAtlas = io.Fonts;
	ImFontConfig arialConfig;
	arialConfig.FontDataOwnedByAtlas = false;
	ImFont* arialFont = fontAtlas->AddFontFromFileTTF("c:\\Windows\\Fonts\\Bahnschrift.ttf", 14.0f, &arialConfig);
	io.Fonts = fontAtlas;
	io.IniFilename = 0;
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowTitleAlign = { 0.5f, 0.5f };
	style->WindowRounding = 0.0f;
	style->WindowBorderSize = 4;
	style->FrameBorderSize = 1;
	style->FramePadding = { 0, 0 };
	style->Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 10);
	style->Colors[ImGuiCol_TitleBg] = ImColor(15, 15, 15);
	style->Colors[ImGuiCol_TitleBgActive] = ImColor(15, 15, 15);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(15, 15, 15);
	style->Colors[ImGuiCol_Border] = ImColor(25, 25, 25);
	style->Colors[ImGuiCol_ChildBg] = ImColor(15, 15, 15);
	style->Colors[ImGuiCol_FrameBg] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_FrameBgActive] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_FrameBgHovered] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_PopupBg] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_Header] = ImColor(0, 0, 0, 0);
	style->Colors[ImGuiCol_HeaderHovered] = ImColor(0, 0, 0, 0);
	style->Colors[ImGuiCol_HeaderActive] = ImColor(0, 0, 0, 0);
	style->Colors[ImGuiCol_CheckMark] = ImColor(0, 128, 255);
	style->Colors[ImGuiCol_SliderGrab] = ImColor(0, 128, 255);
	style->Colors[ImGuiCol_Button] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_Separator] = ImColor(25, 25, 25);
	style->Colors[ImGuiCol_SeparatorActive] = ImColor(25, 25, 25);
	style->Colors[ImGuiCol_SeparatorHovered] = ImColor(25, 25, 25);
	p_object->Release();
	return S_OK;
}
static inline bool find_discord_overlay_window(HWND& hwnd_out) {
	HWND hwnd = nullptr;

	while ((hwnd = FindWindowExA(nullptr, hwnd, "Chrome_WidgetWin_1", nullptr)) != nullptr) {
		DWORD pid = 0;
		GetWindowThreadProcessId(hwnd, &pid);

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (hProcess) {
			char exePath[MAX_PATH];
			if (GetModuleFileNameExA(hProcess, nullptr, exePath, MAX_PATH)) {
				std::string path = exePath;
				if (path.find("Discord") != std::string::npos) {
					hwnd_out = hwnd;
					CloseHandle(hProcess);
					return true;
				}
			}
			CloseHandle(hProcess);
		}
	}
	return false;
}
void overlay()
{
	HWND hwnd_out = nullptr;
	if (!find_discord_overlay_window(hwnd_out)) {
		MessageBoxA(0, "Couldn't find Discord overlay window.", "Error", MB_ICONERROR);
		return;
	}
	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(hwnd_out, &margin);
	SetWindowLong(hwnd_out, GWL_EXSTYLE, GetWindowLong(hwnd_out, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	SetLayeredWindowAttributes(hwnd_out, RGB(0, 0, 0), 255, LWA_ALPHA);
	ShowWindow(hwnd_out, SW_SHOW);
	UpdateWindow(hwnd_out);
	MyWnd = hwnd_out;
}
void show_menu()
{
	char watermarkText[64];
	sprintf_s(watermarkText, "Landens Base | %.i FPS", getFps());
	ImVec2 wmTextSize = ImGui::CalcTextSize(watermarkText);
	ImVec2 rectSize = ImVec2(wmTextSize.x + 2 * 10.0f, wmTextSize.y + 2 * 5.0f);
	ImVec2 rectPos = ImVec2(5, 5);
	ImGui::GetForegroundDrawList()->AddRectFilled(rectPos, ImVec2(rectPos.x + rectSize.x, rectPos.y + rectSize.y), ImColor(10, 10, 10), 0, 0);
	ImGui::GetForegroundDrawList()->AddRectFilled(rectPos, ImVec2(rectPos.x + rectSize.x, rectPos.y + 2), ImColor(0, 128, 255), 0, 0);
	ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), 14.0f, ImVec2(rectPos.x + (rectSize.x - wmTextSize.x) / 2, rectPos.y + (rectSize.y - wmTextSize.y) / 2), ImColor(255, 255, 255), watermarkText);
	if (GetAsyncKeyState(VK_INSERT) & 1) get_menu = !get_menu;
	if (get_menu)
	{
		ImGui::SetNextWindowSize({ 800, 500 });
		ImGui::Begin("PK-Cheat v1", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
		if (ImGui::BeginTabBar("Aimbot")) {
			if (ImGui::BeginTabItem("Aimbot")) {
				ImGui::Checkbox("Enable Aimbot", &settings::aimbot::enable);
				if (settings::aimbot::enable) {
					ImGui::Checkbox("FOV-Kreis anzeigen", &settings::aimbot::show_fov);
					ImGui::ColorEdit3("FOV Farbe", (float*)&settings::colors::icFovColor);
					ImGui::SliderFloat("Aimbot FOV", &settings::aimbot::fov, 10.0f, 180.0f, "%.1f");
					ImGui::SliderFloat("Aimbot Smoothness", &settings::aimbot::smooth, 1.0f, 20.0f, "%.1f");
					ImGui::Combo("Aimbot Ziel", &settings::aimbot::bone, aimbot_bone_names, IM_ARRAYSIZE(aimbot_bone_names));
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("ESP")) {
				ImGui::Checkbox("Enable Visuals", &settings::visuals::enable);
				if (settings::visuals::enable)
				{
					ImGui::Checkbox("Name", &settings::visuals::name);
					ImGui::Checkbox("selfesp", &settings::visuals::selfesp);
					ImGui::Checkbox("Platform", &settings::visuals::platform);
					ImGui::Checkbox("Distance", &settings::visuals::distance);
					ImGui::Checkbox("Rank", &settings::visuals::rank);
					ImGui::Checkbox("Skeleton", &settings::visuals::skeleton);
					ImGui::Checkbox("debug", &settings::visuals::debug);
					ImGui::Checkbox("Box", &settings::visuals::box);
					ImGui::SliderFloat("Render Distance", &settings::visuals::renderDistance, 100, 1000, "%.fm");
					if (ImGui::BeginCombo("Box Type", getBoxTypeName(settings::visuals::boxType)))
					{
						for (auto boxType : boxValues)
						{
							if (ImGui::Selectable(getBoxTypeName(boxType), settings::visuals::boxType == boxType)) settings::visuals::boxType = boxType;
						}
						ImGui::EndCombo();
					}
					ImGui::Separator();
					ImGui::Text("Farben");
					ImGui::ColorEdit3("Box sichtbar", (float*)&settings::colors::icBoxColorVisible);
					ImGui::ColorEdit3("Skelett sichtbar", (float*)&settings::colors::icSkeletonColorVisible);
					ImGui::ColorEdit3("Tracer sichtbar", (float*)&settings::colors::icTracerColorVisible);
					ImGui::ColorEdit3("Fadenkreuz", (float*)&settings::colors::icCrosshairColor);
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("World")) {
				ImGui::Checkbox("Enable World", &settings::world::enableOutlinesLoot);
				if (settings::world::enableOutlinesLoot) {
					ImGui::Checkbox("Uncommon Loot", &settings::world::uncommon_loot);
					ImGui::Checkbox("Common Loot", &settings::world::common_loot);
					ImGui::Checkbox("Rare Loot", &settings::world::rare_loot);
					ImGui::Checkbox("Epic Loot", &settings::world::epic_loot);
					ImGui::Checkbox("Legendary Loot", &settings::world::legendary_loot);
					ImGui::Checkbox("Mythic Loot", &settings::world::mythic_loot);
					ImGui::Checkbox("Chests", &settings::world::chests);
					ImGui::Checkbox("Ammobox", &settings::world::ammobox);
					ImGui::Checkbox("Llamas", &settings::world::llamas);
					ImGui::Checkbox("Foodbox", &settings::world::foodbox);
					ImGui::Checkbox("Vehicle", &settings::world::vehicle);
					ImGui::Checkbox("Spirits", &settings::world::spirits);
					ImGui::SliderInt("Loot Render Distance", &settings::world::render_distance_loot, 50, 1000);
					ImGui::SliderInt("Ammo Render Distance", &settings::world::render_distance_ammo, 50, 1000);
					ImGui::SliderInt("Vehicle Render Distance", &settings::world::render_distance_vehicle, 50, 1000);
					ImGui::SliderInt("Spirits Render Distance", &settings::world::render_distance_spirits, 50, 1000);
					ImGui::SliderInt("Llama Render Distance", &settings::world::render_distance_llama, 50, 1000);
					ImGui::SliderInt("Chest Render Distance", &settings::world::render_distance_chest, 50, 1000);
					ImGui::Checkbox("Outlines Containers", &settings::world::enableOutlinesContainers);
					ImGui::Checkbox("Outlines Loot", &settings::world::enableOutlinesLoot);
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}
}
HWND GetProcWindow(uint32_t pid)
{
	std::pair<HWND, uint32_t> params = { 0, pid };
	BOOL bresult = EnumWindows([](HWND hwnd, LPARAM lparam) -> BOOL
	{
		auto pparams = (std::pair<HWND, uint32_t>*)(lparam);
		uint32_t processid = 0;
		if (GetWindowThreadProcessId(hwnd, reinterpret_cast<LPDWORD>(&processid)) && processid == pparams->second)
		{
			SetLastError((uint32_t)-1);
			pparams->first = hwnd;
			return FALSE;
		}
		return TRUE;
	}, (LPARAM)&params);
	if (!bresult && GetLastError() == -1 && params.first) return params.first;
	return 0;
}

void draw_aimbot_fov_circle() {
	if (!settings::aimbot::show_fov) return;
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	float center_x = width / 2.0f;
	float center_y = height / 2.0f;
	float radius = settings::aimbot::fov;
	drawList->AddCircle(ImVec2(center_x, center_y), radius, IM_COL32(0, 128, 255, 180), 128, 2.0f);
}

WPARAM RenderLoop()
{
	static RECT old_rc;
	ZeroMemory(&messager, sizeof(MSG));
	while (messager.message != WM_QUIT)
	{
		if (PeekMessage(&messager, MyWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&messager);
			DispatchMessage(&messager);
		}
		if (MyWnd == NULL) exit(0);
		HWND active_wnd = GetForegroundWindow();
		RECT rc;
		POINT xy;
		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		if (MyWnd != NULL) {
			GetWindowRect(MyWnd, &rc);
			xy.x = rc.left;
			xy.y = rc.top;
		}
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = 1.0f / 60.0f;
		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;
		if (GetAsyncKeyState(0x1))
		{
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].y = io.MousePos.y;
		}
		else
		{
			io.MouseDown[0] = false;
		}
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			p_params.BackBufferWidth = width;
			p_params.BackBufferHeight = height;
			SetWindowPos(MyWnd, HWND_TOPMOST, rc.left, rc.top, width, height, SWP_NOREDRAW | SWP_NOACTIVATE | SWP_SHOWWINDOW);
			SetWindowLong(MyWnd, GWL_EXSTYLE, GetWindowLong(MyWnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
			SetLayeredWindowAttributes(MyWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
			p_device->Reset(&p_params);
		}
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		actor();
		world_esp();
		show_menu();
		draw_aimbot_fov_circle();
		run_aimbot();
		ImGui::EndFrame();
		p_device->SetRenderState(D3DRS_ZENABLE, false);
		p_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		p_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		p_device->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		if (p_device->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			p_device->EndScene();
		}
		HRESULT result = p_device->Present(0, 0, 0, 0);
		if (result == D3DERR_DEVICELOST && p_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_device->Reset(&p_params);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (p_device != 0)
	{
		p_device->EndScene();
		p_device->Release();
	}
	if (p_object != 0) p_object->Release();
	DestroyWindow(MyWnd);
	return messager.wParam;
}



