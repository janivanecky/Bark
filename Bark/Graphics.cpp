#include "Graphics.h"
#include "Core\String.h"

#define CHECK_WIN_ERROR(x,y) if(FAILED(x)) {OutputDebugStringA(y); return;}
#define RELEASE_DX_RESOURCE(x) if(x){x->Release(); x = NULL;}

void Graphics::ClearTarget(Renderer *renderer, RenderTarget *renderTarget)
{
	float color[] = {0.2f, 0.2f, 0.2f,1};
	renderer->context->ClearRenderTargetView(renderTarget->renderTarget, color);
	renderer->context->ClearDepthStencilView(renderTarget->depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Graphics::InitConstantBuffer(Renderer *renderer, ConstantBuffer *buffer, uint32 dataSize)
{
	uint32 bufferSize = (((dataSize - 1) / 16) + 1) * 16;
	CD3D11_BUFFER_DESC constantBufferDesc(bufferSize, D3D11_BIND_CONSTANT_BUFFER);
	HRESULT hr = renderer->device->CreateBuffer(&constantBufferDesc, NULL,
				 &buffer->buffer);
	CHECK_WIN_ERROR(hr, "Error creating Per Scene buffer\n");
}


void Graphics::InitPipeline(Renderer *renderer, Pipeline *pipeline, VertexInputLayout inputLayout,
							FileData *vertexShaderFile, FileData *pixelShaderFile)
{
	ID3D11Device *device = renderer->device;
	HRESULT hr = device->CreateVertexShader(vertexShaderFile->data, vertexShaderFile->size, 
											NULL, &pipeline->vertexShader);
	CHECK_WIN_ERROR(hr, "Error creating vertex shader\n");

	DXGI_FORMAT formatFromType[5] = {DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, 
		DXGI_FORMAT_R32_FLOAT};

	D3D11_INPUT_ELEMENT_DESC *vertexDesc = new D3D11_INPUT_ELEMENT_DESC[inputLayout.inputCount + inputLayout.instanceCount];
	for (uint32 i = 0; i < inputLayout.inputCount; i++)
	{
		D3D11_INPUT_ELEMENT_DESC *desc = &vertexDesc[i];
		desc->SemanticName = inputLayout.names[i];
		desc->SemanticIndex = 0;
		desc->Format = formatFromType[inputLayout.components[i]];
		desc->InputSlot = 0;
		desc->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		desc->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		desc->InstanceDataStepRate = 0;
	}

	uint32 semanticIndex = 0;
	for (uint32 i = 0; i < inputLayout.instanceCount; i++)
	{
		D3D11_INPUT_ELEMENT_DESC *desc = &vertexDesc[inputLayout.inputCount + i];
		if (i > 0 && String::Compare(inputLayout.instanceNames[i], inputLayout.instanceNames[i - 1]))
		{
			++semanticIndex;
		}
		else
		{
			semanticIndex = 0;
		}
		desc->SemanticName = inputLayout.instanceNames[i];
		desc->SemanticIndex = semanticIndex;
		desc->Format = formatFromType[inputLayout.instanceComponents[i]];
		desc->InputSlot = 1;
		desc->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		desc->InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		desc->InstanceDataStepRate = 1;
	}

	hr = device->CreateInputLayout(vertexDesc, inputLayout.inputCount + inputLayout.instanceCount, 
								   vertexShaderFile->data, vertexShaderFile->size,
								   &pipeline->inputLayout);

	delete []vertexDesc;

	CHECK_WIN_ERROR(hr, "Error creating input layout\n");

	if (pixelShaderFile)
	{
		hr = device->CreatePixelShader(pixelShaderFile->data, pixelShaderFile->size, 
									   NULL, &pipeline->pixelShader);
		CHECK_WIN_ERROR(hr, "Error creating pixel shader\n");
	}
}

void Graphics::InitModel(Renderer *renderer, Model *model, float *vertices, uint32 vertexCount, uint32 vertexStride,
						uint16 *indices, int32 indexCount, BufferType type)
{
	ID3D11Device *device = renderer->device;

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = vertices;
	model->vertexStride = vertexStride;
	model->vertexCount = vertexCount;
	model->indexCount = indexCount;
	D3D11_USAGE usage = type == NORMAL ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
	uint32 cpuAccess = type == NORMAL ? 0 : D3D11_CPU_ACCESS_WRITE;
	CD3D11_BUFFER_DESC vertexBufferDesc(vertexCount * vertexStride, D3D11_BIND_VERTEX_BUFFER, usage, cpuAccess);
	HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &model->vertexBuffer);
	CHECK_WIN_ERROR(hr, "Error creating vertex buffer\n");

	if (indices)
	{
		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = indices;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(uint16) * indexCount, D3D11_BIND_INDEX_BUFFER);
		hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, &model->indexBuffer);
		CHECK_WIN_ERROR(hr, "Error creating index buffer\n");
	}
	else
	{
		model->indexBuffer = NULL;
	}
}

void Graphics::BindTarget(Renderer *renderer, RenderTarget *renderTarget)
{
	renderer->context->OMSetRenderTargets(1, &renderTarget->renderTarget, renderTarget->depthStencil);
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)renderTarget->width, (float)renderTarget->height);
	renderer->context->RSSetViewports(1, &viewport);
}

void Graphics::BindTargetDepthless(Renderer *renderer, RenderTarget *renderTarget)
{
	renderer->context->OMSetRenderTargets(1, &renderTarget->renderTarget, NULL);
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)renderTarget->width, (float)renderTarget->height);
	renderer->context->RSSetViewports(1, &viewport);
}

void Graphics::BindPipeline(Renderer *renderer, Pipeline *pipeline)
{
	renderer->context->VSSetShader(pipeline->vertexShader, NULL, 0);
	renderer->context->IASetInputLayout(pipeline->inputLayout);
	renderer->context->PSSetShader(pipeline->pixelShader, 0, 0);
}

void Graphics::BindConstantBuffer(Renderer *renderer, ConstantBuffer *buffer, void *data, uint32 slot)
{
	renderer->context->UpdateSubresource(buffer->buffer, 0, 0, data, 0, 0);
	renderer->context->VSSetConstantBuffers(slot, 1, &buffer->buffer);
}

D3D11_PRIMITIVE_TOPOLOGY TopologyFromType[] = {
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST
};

void Graphics::RenderModel(Renderer *renderer, Model *model, PrimitiveType type)
{
	ID3D11Buffer *buffers[] = {model->vertexBuffer};
	UINT strides[] = {model->vertexStride};
	UINT offsets[] = {0};
	renderer->context->IASetPrimitiveTopology(TopologyFromType[type]);
	renderer->context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
	if (model->indexBuffer)
	{
		renderer->context->IASetIndexBuffer(model->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		renderer->context->DrawIndexed(model->indexCount, 0, 0);
	}
	else
	{
		renderer->context->Draw(model->vertexCount,0);
	}
}

void Graphics::UpdateModelData(Renderer *renderer, Model *model, void *data, uint32 size, uint32 vertexCount)
{
	if (size > 0 && vertexCount > 0)
	{
		model->vertexCount = vertexCount;
	}
	D3D11_MAPPED_SUBRESOURCE resource = {};
	renderer->context->Map(model->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, size);
	renderer->context->Unmap(model->vertexBuffer, 0);
}

void Graphics::RenderModelInstanced(Renderer *renderer, Model *model, uint32 instanceCount, PrimitiveType type)
{
	ID3D11Buffer *buffers[] = { model->vertexBuffer};
	UINT strides[] = { model->vertexStride };
	UINT offsets[] = { 0 };
	renderer->context->IASetPrimitiveTopology(TopologyFromType[type]);
	renderer->context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
	if (model->indexBuffer)
	{
		renderer->context->IASetIndexBuffer(model->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		renderer->context->DrawIndexedInstanced(model->indexCount, instanceCount, 0, 0, 0);
	}
	else
	{
		renderer->context->DrawInstanced(model->vertexCount, instanceCount, 0, 0);
	}
}

void Graphics::Release(Pipeline *pipeline)
{
	RELEASE_DX_RESOURCE(pipeline->inputLayout);
	RELEASE_DX_RESOURCE(pipeline->vertexShader);
	RELEASE_DX_RESOURCE(pipeline->pixelShader);
}

void Graphics::Release(Model *model)
{
	RELEASE_DX_RESOURCE(model->vertexBuffer);
	RELEASE_DX_RESOURCE(model->indexBuffer);
}

void Graphics::Release(ConstantBuffer *buffer)
{
	RELEASE_DX_RESOURCE(buffer->buffer);
}

void Graphics::CreateWhiteTexture(Renderer *renderer, Texture *texture)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = 1;
	textureDesc.Height = 1;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	uint32 whiteColor = 0xffffffff;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = &whiteColor;
	initialData.SysMemPitch = sizeof(whiteColor) * textureDesc.Width;

	ID3D11Texture2D *tmpTexture2D;
	HRESULT hr = renderer->device->CreateTexture2D(&textureDesc, &initialData, &tmpTexture2D);
	CHECK_WIN_ERROR(hr, "Error creating texture2D.\n");

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = renderer->device->CreateShaderResourceView(tmpTexture2D, &shaderResourceViewDesc, &texture->shaderResourceView);
	RELEASE_DX_RESOURCE(tmpTexture2D);
}

void Graphics::BindTexture(Renderer* renderer, Texture *texture)
{
	renderer->context->PSSetShaderResources(0, 1, &texture->shaderResourceView);
	renderer->context->PSSetSamplers(0, 1, &renderer->texSampler);
}

void Graphics::InitTexture(Renderer *renderer, Texture *texture, TextureDesc textureDesc)
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = textureDesc.width;
	texDesc.Height = textureDesc.height;
	texDesc.MipLevels = textureDesc.mipCount;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA texData[15] = {};
	uint8 *mipData = (uint8 *)textureDesc.data;
	uint32 mipWidth = textureDesc.width;
	uint32 mipHeight = textureDesc.height;
	for (uint32 i = 0; i < ARRAYSIZE(texData) && i < textureDesc.mipCount; i++)
	{
		texData[i].pSysMem = mipData;
		texData[i].SysMemPitch = mipWidth * 4;
		mipData += mipWidth * mipHeight * 4;
		mipWidth /= 2;
		mipHeight /= 2;
	}

	ID3D11Texture2D *tex;
	HRESULT hr = renderer->device->CreateTexture2D(&texDesc, texData, &tex);
	CHECK_WIN_ERROR(hr, "Error creating texture2D\n");

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.MipLevels = -1;

	hr = renderer->device->CreateShaderResourceView(tex, &viewDesc, &texture->shaderResourceView);
	tex->Release();
	CHECK_WIN_ERROR(hr, "Error creating texture view\n");
}