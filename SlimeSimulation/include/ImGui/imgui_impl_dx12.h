
class GraphicsDevice;
class DescriptorHeap;

struct ID3D12GraphicsCommandList;

void ImGui_ImplDX12_Init();
bool ImGui_ImplDX12_CreateDeviceObjects(GraphicsDevice* device);
void ImGui_ImplDX12_CreateFontsTexture(GraphicsDevice* device, DescriptorHeap* heap);
void ImGui_ImplDX12_RenderDrawData(GraphicsDevice* device, ImDrawData* draw_data, ID3D12GraphicsCommandList* ctx);