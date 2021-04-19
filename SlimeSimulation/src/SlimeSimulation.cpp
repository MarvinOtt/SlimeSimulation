#include "SlimeSimulation.h"

#ifdef _DEBUG 
void signalHandler(int signum)
{

	ofstream file;
	file.open("log.txt");
	file << boost::stacktrace::stacktrace();
	file.close();
	msgBox(string("Critical Error: See log file"));
	exit(signum);
}

void my_terminate_handler()
{
	signalHandler(0);
}
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
#ifdef _DEBUG 
	signal(SIGSEGV, signalHandler);
	set_terminate(my_terminate_handler);
#endif

	SetProcessDPIAware();
	srand((UINT)time(NULL));
	if (!Monitor::GetMonitorData())
	{
		msgBox("No monitors found");
		return -1;
	}

	Width = static_cast<int>(Monitor::monitors[0].sizeX * 0.8f);
	Height = static_cast<int>(Monitor::monitors[0].sizeY * 0.8f);

	Setting::Load();
	selectedMonitor = Setting::defaultMonitor;

	//create the window
	mainWindow = new Window(hInstance);
	if (!Setting::StartInBackground)
		mainWindow->Create(false, L"SlimeSimulation", Width, Height, WndProc, 0);
	else
	{
		mainWindow->Create(true, L"SlimeSimulation", Monitor::monitors[selectedMonitor].sizeX, Monitor::monitors[selectedMonitor].sizeY, WndProc, 0);
		mainWindow->Show(SW_HIDE);
	}

	InitInput();
	InitD3D();
	if (Setting::StartInBackground)
		BackgroundHandler::SetBG(true, graphicsDevice, mainWindow, Monitor::monitors[selectedMonitor].sizeX, Monitor::monitors[selectedMonitor].sizeY, Monitor::GetMonPosX(selectedMonitor), Monitor::GetMonPosY(selectedMonitor));

	mainWindow->Show(SW_SHOW);
	IsInitDone = true;


	// Main loop
	while (true)
	{
		if (!Update())
			break;
		Render();
	}
	return 0;
}

bool InitInput()
{
	keyboardstate = new char[256];
	oldkeyboardstate = new char[256];
	return true;
}

bool InitD3D()
{

	graphicsDevice = new GraphicsDevice(mainWindow->hwnd, Width, Height);
	graphicsDevice->Create();

	mainHeap = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	mainHeap->Create(graphicsDevice, 1 + 3);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(mainWindow->hwnd);
	ImGui_ImplDX12_Init();
	ImGui_ImplDX12_CreateDeviceObjects(graphicsDevice);
	ImGui_ImplDX12_CreateFontsTexture(graphicsDevice, mainHeap);

	// Main D3D

	NewTexture3D_Create(particletex, graphicsDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, 64, 64, particleMul);
	NewTexture2D_Create(trailtex1, graphicsDevice, DXGI_FORMAT_R16_FLOAT, Monitor::maxMonWidth, Monitor::maxMonHeight);
	NewTexture2D_Create(trailtex2, graphicsDevice, DXGI_FORMAT_R16_FLOAT, Monitor::maxMonWidth, Monitor::maxMonHeight);
	
	particleBufferCPU = new BYTE[particletex->bufferSize];
	std::uniform_int_distribution<int> rngdist(0, 628318);
	std::random_device rng;
	for (int i = 0; i < 64 * 64 * particleMul; i++)
	{
		XMFLOAT2* curpos = (XMFLOAT2*)(particleBufferCPU + i * 16 + 0);
		float* curangle = (float*)(particleBufferCPU + i * 16 + 8);
		curpos[0] = XMFLOAT2((float)(rand() % 400 + 700), (float)(rand() % 400 + 400));
		float val = (((float)(rngdist(rng))) / 100000.0f);
		curangle[0] = val;
	}
	particletex->SetData(graphicsDevice, particleBufferCPU, (int)particletex->bufferSize);

	mainHeap->SetTexture3D(particletex, 1);
	mainHeap->SetTexture2D(trailtex1, 2);
	mainHeap->SetTexture2D(trailtex2, 3);

	// Root Signatures

	particleRS = new DXR_RootSignature();
	std::vector<DescriptorRange> particle_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 1} };
	particleRS->Add_Sampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0);
	particleRS->AddRP_CBV(0);
	particleRS->AddRP_DescriptorTable(particle_DR);
	particleRS->CreateWithFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	particleRS->Build(graphicsDevice);

	diffuseDecayRS = new DXR_RootSignature();
	std::vector<DescriptorRange> RS_compute_DR{ {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0}, {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3, 1} };
	diffuseDecayRS->AddRP_CBV(0);
	diffuseDecayRS->AddRP_DescriptorTable(RS_compute_DR);
	diffuseDecayRS->CreateWithFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	diffuseDecayRS->Build(graphicsDevice);
	
	// Shaders and Pipelines

	diffuseDecayVS = new Shader();
	diffuseDecayVS->Load(L"Shaders/BasicVertexShader.hlsl", "main", "vs_5_0");

	shaderDD_1TrailMaps = new Shader();
	shaderDD_1TrailMaps->Load(L"Shaders/DD_1TrailMaps.hlsl", "main", "ps_5_0");

	shaderDD_2TrailMaps = new Shader();
	shaderDD_2TrailMaps->Load(L"Shaders/DD_2TrailMaps.hlsl", "main", "ps_5_0");

	pipelineStateDD1 = new PipelineState();
	pipelineStateDD1->SetShaders(diffuseDecayVS, shaderDD_1TrailMaps);
	pipelineStateDD1->AddRTV(DXGI_FORMAT_R8G8B8A8_UNORM);
	pipelineStateDD1->Create(graphicsDevice, inputLayout, (int)sizeof(inputLayout), diffuseDecayRS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	pipelineStateDD2 = new PipelineState();
	pipelineStateDD2->SetShaders(diffuseDecayVS, shaderDD_2TrailMaps);
	pipelineStateDD2->AddRTV(DXGI_FORMAT_R8G8B8A8_UNORM);
	pipelineStateDD2->Create(graphicsDevice, inputLayout, (int)sizeof(inputLayout), diffuseDecayRS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	for (int i = 0; i < Setting::numRules; ++i)
	{
		Shader* curParticleShader = new Shader();
		curParticleShader->Load((wstring(L"Shaders/ParticleRule_") + std::to_wstring(i) + wstring(L".hlsl")).c_str(), "main", "cs_5_0");

		PipelineState* curPipelineState = new PipelineState();
		curPipelineState->SetCS(curParticleShader);
		curPipelineState->CreateCompute(graphicsDevice, particleRS);

		Setting::rules[i].diffuseDecayPipelineState = Setting::rules[i].particleCount == 1 ? pipelineStateDD1 : pipelineStateDD2;
		Setting::rules[i].particleShader = curParticleShader;
		Setting::rules[i].particlePipelineState = curPipelineState;
	}

	// Constant Buffers

	particleCB = new ConstantBuffer<particleCB_DEF>();
	particleCB_DEF particle_DEF;
	particle_DEF.sensor_angle = Setting::CurSett().sensor_angle;
	particle_DEF.sensor_distance = Setting::CurSett().sensor_distance;
	particle_DEF.steeringangle = Setting::CurSett().steeringangle;
	particle_DEF.walkingdistance = Setting::CurSett().walkingdistance;
	particle_DEF.depositStrength = Setting::CurSett().depositStrength;
	particle_DEF.resX = static_cast<float>(Monitor::maxMonWidth);
	particle_DEF.resY = static_cast<float>(Monitor::maxMonHeight);
	particleCB->Create(graphicsDevice);
	particleCB->cb = particle_DEF;

	diffuseDecayCB = new ConstantBuffer<diffuseDecayCB_DEF>();
	diffuseDecayCB_DEF diffuseDecay_DEF;
	diffuseDecay_DEF.diffuseStrength = Setting::CurSett().diffuseStrength;
	diffuseDecay_DEF.decayStrength = Setting::CurSett().decayStrength;
	diffuseDecay_DEF.partCol1 = Setting::partCol1 = Setting::singleColors[0];
	diffuseDecayCB->Create(graphicsDevice);
	diffuseDecayCB->cb = diffuseDecay_DEF;

	fullscreenBuffer = new VertexBuffer<DirectX::XMFLOAT4>();
	fullscreenBuffer->CreateFullScreen(graphicsDevice);

	graphicsDevice->commandList->CloseAndExecute(graphicsDevice->commandQueue);
	graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue++;
	d3d_call(graphicsDevice->commandQueue->commandQueue->Signal(graphicsDevice->fences[graphicsDevice->frameIndex].fence, graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue));

	lastSettingSwitch = high_resolution_clock::now();
	ZeroMemory(&msg, sizeof(MSG));
	return true;
}

bool Update()
{
	resFac = 2359296.0f / (float)(graphicsDevice->bufferWidth * graphicsDevice->bufferHeight);
	GetKeyboardState((PBYTE)keyboardstate);

	particleCB->cb.curResX = static_cast<float>(graphicsDevice->bufferWidth);
	particleCB->cb.curResY = static_cast<float>(graphicsDevice->bufferHeight - (BackgroundHandler::IsBackground ? 40 * Monitor::monitors[selectedMonitor].scaling : 0));
	diffuseDecayCB->cb.backgroundColor = Setting::backgroundColor;

	auto curTimeStamp = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(curTimeStamp - lastSettingSwitch);
	int millisecondsDiff = (int)duration.count();
	progress = millisecondsDiff / (float)Setting::switchTime;
	if (millisecondsDiff > Setting::switchTime && IsAutoRotate)
	{
		lastSettingSwitch = high_resolution_clock::now();
		int oldParticleCount = Setting::CurRule().particleCount;
		Setting::GenerateNewRuleSetting(0.4f);
		if(oldParticleCount != Setting::CurRule().particleCount)
			ChangeTrailTextures(Setting::CurRule().particleCount);
		Setting::StartTransition();
	}
	Setting::Update(resFac, &particleCB->cb, &diffuseDecayCB->cb);

	memcpy(diffuseDecayCB->cbvGPUAddress[graphicsDevice->frameIndex], &diffuseDecayCB->cb, sizeof(diffuseDecayCB_DEF));
	memcpy(particleCB->cbvGPUAddress[graphicsDevice->frameIndex], &particleCB->cb, sizeof(particleCB_DEF));

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void ChangeTrailTextures(int particleCount)
{
	graphicsDevice->WaitForEventCompletion(graphicsDevice->frameIndex);
	delete trailtex1;
	delete trailtex2;
	NewTexture2D_Create(trailtex1, graphicsDevice, particleCount == 1 ? DXGI_FORMAT_R16_FLOAT : DXGI_FORMAT_R16G16_FLOAT, Monitor::maxMonWidth, Monitor::maxMonHeight);
	NewTexture2D_Create(trailtex2, graphicsDevice, particleCount == 1 ? DXGI_FORMAT_R16_FLOAT : DXGI_FORMAT_R16G16_FLOAT, Monitor::maxMonWidth, Monitor::maxMonHeight);
	mainHeap->SetTexture2D(trailtex1, 2);
	mainHeap->SetTexture2D(trailtex2, 3);
}

void PrepareSettingsWnd()
{
	ImGui::Begin("Settings");

	ImGui::SliderFloat("Sens Angle", &particleCB->cb.sensor_angle, 0.0f, 3.1415926f, "%.5f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Sens Dist", &particleCB->cb.sensor_distance, 0.5f, 20.0f);
	ImGui::SliderFloat("Steering Angle", &particleCB->cb.steeringangle, 0.005f, 1.0f);
	ImGui::SliderFloat("Walking Dist", &particleCB->cb.walkingdistance, 0.05f, 5.0f);
	ImGui::SliderFloat("Deposit Strength", &particleCB->cb.depositStrength, 0.005f, 0.5f);
	ImGui::Separator();
	ImGui::SliderFloat("Diff. Strength", &diffuseDecayCB->cb.diffuseStrength, 0.0f, 1.0f);
	ImGui::SliderFloat("Decay Strength", &diffuseDecayCB->cb.decayStrength, 0.0f, 1.0f * resFac, "%.5f", ImGuiSliderFlags_Logarithmic);
	ImGui::Separator();
	if (ImGui::Button(BackgroundHandler::IsBackground ? "Set Windowed" : "Set Background"))
		BackgroundHandler::SetBG(true, graphicsDevice, mainWindow, Monitor::monitors[selectedMonitor].sizeX, Monitor::monitors[selectedMonitor].sizeY, Monitor::GetMonPosX(selectedMonitor), Monitor::GetMonPosY(selectedMonitor));
	ImGui::SameLine();
	if (ImGui::Button("Copy2Clip"))
		Setting::Copy2Clip(resFac, mainWindow->hwnd, &particleCB->cb, &diffuseDecayCB->cb);
	ImGui::SameLine();
	if (ImGui::Button("Advanced"))
		ShowAdvanced = !ShowAdvanced;

	if (ImGui::Checkbox("Rotate Settings", &IsAutoRotate))
	{
		lastSettingSwitch = high_resolution_clock::now();
		progress = 0;
	}
	ImGui::SameLine();
	ImGui::ProgressBar(IsAutoRotate ? progress : 0.0f);
	int rule = Setting::curRule;
	if (ImGui::InputInt("Cur Rule", &rule))
	{
		lastSettingSwitch = high_resolution_clock::now();
		rule = std::clamp(rule, 0, Setting::numRules - 1);
		int newParticleCount = Setting::rules[rule].particleCount;
		if (Setting::CurRule().particleCount != newParticleCount)
			ChangeTrailTextures(newParticleCount);
		Setting::curRule = rule;
		Setting::curSetting = std::clamp(Setting::curSetting, 0, (int)Setting::CurRule().settings.size() - 1);
		Setting::GenerateNewColor();
		Setting::StartTransition();
		progress = 0;
	}
	if (ImGui::InputInt("Cur Setting", &Setting::curSetting))
	{
		Setting::curSetting = std::clamp(Setting::curSetting, 0, (int)Setting::CurRule().settings.size() - 1);
		Setting::GenerateNewColor();
		Setting::StartTransition();
		lastSettingSwitch = high_resolution_clock::now();
		progress = 0;
	}

	ImGui::End();

	if (ShowAdvanced)
		PrepareAdvancedWnd();
}

void PrepareAdvancedWnd()
{
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
	ImGui::Begin("Advanced Settings", nullptr, Monitor::IsMonitorHovered ? ImGuiWindowFlags_NoMove : ImGuiWindowFlags_None);

	ImGui::Text("Monitors:");
	Monitor::ImGui_WndPreview(350.0f, selectedMonitor);
	
	ImGui::Spacing();
	ImGui::Text(("Selected Monitor: " + std::to_string(selectedMonitor) + " (" + std::to_string(Monitor::monitors[selectedMonitor].sizeX) + " x " + std::to_string(Monitor::monitors[selectedMonitor].sizeY) + ")").c_str());
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::InputInt("V Sync Factor", &Setting::VSyncFactor))
		Setting::VSyncFactor = std::clamp(Setting::VSyncFactor, 0, 4);
	if (ImGui::InputInt("Change Interval", &Setting::switchTime, 100, 1000))
		Setting::switchTime = std::clamp(Setting::switchTime, 500, 1000000);

	ImGui::ColorEdit3("Background Color", (float*)&Setting::backgroundColor);
	ImGui::End();
}

void PrepareGUI()
{
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (!BackgroundHandler::IsBackground)
		PrepareSettingsWnd();
	if ((keyboardstate[VK_CONTROL] & 0x8000) && (keyboardstate[VK_MENU] & 0x8000) && (keyboardstate[0x47] & 0x8000) && BackgroundHandler::IsBackground)
		BackgroundHandler::SetBG(false, graphicsDevice, mainWindow, Width, Height, 100, 100);

	ImGui::Render();

}

void UpdatePipeline()
{
	graphicsDevice->WaitForEventCompletion(graphicsDevice->frameIndex);
	graphicsDevice->frameIndex = graphicsDevice->swapChain->swapChain->GetCurrentBackBufferIndex();
	graphicsDevice->ResetCommandList();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(graphicsDevice->rtvHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart(), graphicsDevice->frameIndex, graphicsDevice->rtvHeap->descriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(graphicsDevice->dsHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart());

	graphicsDevice->commandList->commandList->SetPipelineState(Setting::CurRule().particlePipelineState->pipelineState);
	graphicsDevice->commandList->commandList->SetComputeRootSignature(particleRS->RS);

	graphicsDevice->commandList->commandList->SetDescriptorHeaps(1, &mainHeap->descriptorHeap);
	graphicsDevice->commandList->commandList->SetComputeRootDescriptorTable(1, mainHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	graphicsDevice->commandList->commandList->SetComputeRootConstantBufferView(0, particleCB->constantBufferUploadHeaps[graphicsDevice->frameIndex]->GetGPUVirtualAddress());

	graphicsDevice->commandList->commandList->Dispatch(64, 1, 1);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = trailtex1->buffer;
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &uavBarrier);

	graphicsDevice->commandList->ResourceBarrierTransition(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, graphicsDevice->backBuffer->renderTargets[graphicsDevice->frameIndex]);

	graphicsDevice->commandList->commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	graphicsDevice->commandList->commandList->RSSetViewports(1, &graphicsDevice->viewport);

	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	graphicsDevice->commandList->commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	graphicsDevice->commandList->commandList->SetDescriptorHeaps(1, &mainHeap->descriptorHeap);

	// Draw Slimes
	graphicsDevice->commandList->commandList->SetPipelineState(Setting::CurRule().diffuseDecayPipelineState->pipelineState);
	graphicsDevice->commandList->commandList->SetGraphicsRootSignature(diffuseDecayRS->RS);
	graphicsDevice->commandList->commandList->SetGraphicsRootConstantBufferView(0, diffuseDecayCB->constantBufferUploadHeaps[graphicsDevice->frameIndex]->GetGPUVirtualAddress());
	graphicsDevice->commandList->commandList->SetGraphicsRootDescriptorTable(1, mainHeap->descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	graphicsDevice->commandList->commandList->RSSetViewports(1, &graphicsDevice->viewport);
	graphicsDevice->commandList->commandList->RSSetScissorRects(1, &graphicsDevice->scissorRect);
	graphicsDevice->commandList->commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphicsDevice->commandList->commandList->IASetVertexBuffers(0, 1, &fullscreenBuffer->bufferView);

	graphicsDevice->commandList->commandList->DrawInstanced(6, 1, 0, 0);

	uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = trailtex2->buffer;
	graphicsDevice->commandList->commandList->ResourceBarrier(1, &uavBarrier);

	graphicsDevice->commandList->commandList->CopyResource(trailtex1->buffer, trailtex2->buffer);

	// Draw GUI
	ImGui_ImplDX12_RenderDrawData(graphicsDevice, ImGui::GetDrawData(), graphicsDevice->commandList->commandList);

	graphicsDevice->commandList->ResourceBarrierTransition(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, graphicsDevice->backBuffer->renderTargets[graphicsDevice->frameIndex]);

	graphicsDevice->commandList->CloseAndExecute(graphicsDevice->commandQueue);
	graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue++;
	graphicsDevice->commandQueue->commandQueue->Signal(graphicsDevice->fences[graphicsDevice->frameIndex].fence, graphicsDevice->fences[graphicsDevice->frameIndex].fenceValue);
}

void Render()
{
	PrepareGUI();

	UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

	d3d_call(graphicsDevice->swapChain->swapChain->Present(Setting::VSyncFactor, 0)); // Present current frame

	char* buf = oldkeyboardstate;
	oldkeyboardstate = keyboardstate;
	keyboardstate = oldkeyboardstate;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	switch (msg)
	{
	case WM_SIZE:
	{
		if (!IsInitDone)
			break;
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		if (width == 0 || height == 0)
			break;
		mainWindow->Resize(width, height);
		graphicsDevice->ResizeBackBuffer(width, height);
		Setting::StartTransition();
	}
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
