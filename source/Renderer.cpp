//External includes
#include "SDL.h"
#include "SDL_surface.h"

#include <thread>

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <future> // ASYNC stuff

#include <ppl.h> //Parallel stuff


//#define ASYNC
#define PARALLEL_FOR

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const uint32_t numPixels = m_Width * m_Height;

	float aspectRatio{ m_Width / float(m_Height) };

	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };
	const float fov{ tan(camera.fovAngle * TO_RADIANS / 2.f) };	


#if defined(ASYNC)
	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_futures{};
	const uint32_t numPixelsPerTask = numPixels / numCores;
	uint32_t numUnassigndPixels = numPixels % numCores;
	uint32_t currPixelIndex{ 0 };


	for (uint32_t coreId = 0; coreId < numCores; ++coreId)
	{
		uint32_t taskSize{ numPixelsPerTask };
		if (numUnassigndPixels)
		{
			++taskSize;
			--numUnassigndPixels;
		}

		async_futures.push_back(std::async(std::launch::async, [=, this]
			{
				const uint32_t pixelIndexEnd = currPixelIndex + taskSize;
				for (uint32_t pixelIndex{ currPixelIndex }; pixelIndex < pixelIndexEnd; pixelIndex++)
				{
					RenderPixel(pScene, pixelIndex, fov, aspectRatio, camera, lights, materials);
				}
			}));
		currPixelIndex += taskSize;
	}

	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}


#elif defined(PARALLEL_FOR)

	concurrency::parallel_for(0u, numPixels, [=, this](int i) {
		RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
		});	

#else
	//sychroon

	for (uint32_t i = 0; i < numPixels; i++)
	{
		RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
	}
#endif


	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}


	


bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}


void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, 
	const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px = pixelIndex % m_Width ;
	const int py = pixelIndex / m_Width ;

	float rx{ px + 0.5f };
	float ry{ py + 0.5f };

	float gradient = px / static_cast<float>(m_Width);
	gradient += py / static_cast<float>(m_Width);
	gradient /= 2.0f;

	float cx = ((2 * (px + 0.5f)) / m_Width - 1) * aspectRatio;
	float cy = 1 - (2 * (py + 0.5f)) / m_Height;

	Vector3 rayDirection{ cx,cy,1 };
	rayDirection.Normalize();

	rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
	rayDirection.Normalize();
	
	Ray viewRay{};
	HitRecord closestHit{};
	ColorRGB finalColor{};	

	viewRay = { camera.origin, rayDirection };
	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		for (const Light& light : lights)
		{
			Vector3 lightDirection = LightUtils::GetDirectionToLight(light, closestHit.origin + (closestHit.normal * 0.001f)).Normalized();
			const float lightrayMagnitude{ lightDirection.Magnitude() };
			if (m_ShadowsEnabled)
			{
				Ray lightRay{ closestHit.origin + (closestHit.normal * 0.001f),lightDirection };
				lightRay.max = lightrayMagnitude;
				if (pScene->DoesHit(lightRay))
				{
					continue;
				}
			}

			const float observedArea{ Vector3::Dot(closestHit.normal, lightDirection) };

			switch (m_CurrentLightingMode)
			{
			case LightingMode::Combined:
				if (observedArea > 0.f)
				{
					finalColor += (LightUtils::GetRadiance(light, closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, rayDirection)) * observedArea;
				}
				break;
			case LightingMode::Radiance:
				finalColor += LightUtils::GetRadiance(light, closestHit.origin);
				break;

			case LightingMode::BRDF:
				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, rayDirection);
				break;

			case LightingMode::ObservedArea:
				if (observedArea > 0.f)
				{
					finalColor += ColorRGB{ 1.f, 1.f, 1.f } *observedArea;
				}
				break;
			}
		}
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}


