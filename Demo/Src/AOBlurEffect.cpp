#include "pch.h"
#include "Shaders.h"

namespace
{
#include "Shaders\Compiled\DeferredScreenQuadVS.h"
#include "Shaders\Compiled\AmbientBlurPS.h"
}

using namespace DirectX;

void AOBlurEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_DeferredScreenQuadVS, sizeof(g_DeferredScreenQuadVS),
		g_AmbientBlurPS, sizeof(g_AmbientBlurPS));

	m_CbPerFrame.Create(device);
}

void AOBlurEffect::SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv)
{
	context->PSSetShaderResources(0, bufferCount, srv);
}

void AOBlurEffect::SetInputMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	context->PSSetShaderResources(4, 1, &srv);
}

void AOBlurEffect::SetBlurWeights(float sigma)
{
}

void AOBlurEffect::SetHorizontalBlur(bool isHorizontal)
{
	if (isHorizontal)
		m_CbPerFrameData.HorizontalBlur = 1.0f;
	else
		m_CbPerFrameData.HorizontalBlur = -1.0f;
}

void AOBlurEffect::SetScreenResolution(float width, float height)
{
	m_CbPerFrameData.width = width;
	m_CbPerFrameData.height = height;
}

void AOBlurEffect::SetProjMatrix(CXMMATRIX proj)
{
    m_CbPerFrameData.Proj = proj;
}

void AOBlurEffect::SetBlurEdgePreserve(bool isEdgePreserving)
{
    if (isEdgePreserving)
        m_CbPerFrameData.preserveEdge = 1.0f;
    else
        m_CbPerFrameData.preserveEdge = -1.0f;
}

void AOBlurEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}

void AOBlurEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);
	context->PSSetConstantBuffers(0, 1, m_CbPerFrame.GetAddressOf());
}

void AOBlurEffect::CalculateBlurWeights(float sigma)
{
    float twoSigma2 = 2.0f * sigma * sigma;

    // Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
    // For example, for sigma = 3, the width of the bell curve is 
    int blurRadius = (int)ceil(2.0f * sigma);

    assert(blurRadius <= 5);

    m_BlurWeights.resize(2 * blurRadius + 1);

    float weightSum = 0.0f;

    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        float x = (float)i;

        m_BlurWeights[i + blurRadius] = expf(-x * x / twoSigma2);

        weightSum += m_BlurWeights[i + blurRadius];
    }

    // Divide by the sum so all the weights add up to 1.0.
    for (int i = 0; i < m_BlurWeights.size(); ++i)
    {
        m_BlurWeights[i] /= weightSum;
    }

    m_CbPerFrameData.BlurWeights[0] = XMFLOAT4(&m_BlurWeights[0]);
    m_CbPerFrameData.BlurWeights[1] = XMFLOAT4(&m_BlurWeights[4]);
    m_CbPerFrameData.BlurWeights[2] = XMFLOAT4(&m_BlurWeights[8]);
}
