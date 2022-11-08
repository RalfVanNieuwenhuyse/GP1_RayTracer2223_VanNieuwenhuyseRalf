#pragma once

#include <cstdint>
#include<vector>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;
	struct Camera;
	struct Light;
	class Material;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;

		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera,
			const std::vector<Light>& lights, const std::vector<Material*>& materials)const;

		bool SaveBufferToImage() const;


		void Toggelshadow() { m_ShadowsEnabled = !m_ShadowsEnabled; }

		void CycleLightingModes() {
			switch (m_CurrentLightingMode)
			{
			case dae::Renderer::LightingMode::ObservedArea:
				m_CurrentLightingMode = LightingMode::Radiance;
				break;
			case dae::Renderer::LightingMode::Radiance:
				m_CurrentLightingMode = LightingMode::BRDF;
				break;
			case dae::Renderer::LightingMode::BRDF:
				m_CurrentLightingMode = LightingMode::Combined;
				break;
			case dae::Renderer::LightingMode::Combined:
				m_CurrentLightingMode = LightingMode::ObservedArea;
				break;
			default:
				break;
			}
		}		

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BRDF,
			Combined
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true };

		

	};
}

