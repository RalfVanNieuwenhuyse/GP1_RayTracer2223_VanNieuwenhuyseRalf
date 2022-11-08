#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}			
		{
		}

		
		Vector3 origin{};		

		float fovAngle{90.f};
		float previosFovAngle{};

		Vector3 forward{Vector3::UnitZ};
		//Vector3 forward{ 0.266f,-0.453f, 0.860f};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};
		

		float totalPitch{0.f};
		float totalYaw{0.f};
		

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			//Matrix cameraONB;
			Vector3 worldUp{ 0, 1, 0 };
			

			right = Vector3::Cross(worldUp, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			cameraToWorld = { right, up, forward, origin };
			return cameraToWorld;
			/*assert(false && "Not Implemented Yet");
			return {};*/
		}

		void ChangeFovAngle(float changeAngle)
		{
			fovAngle += changeAngle;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float fovAngleChange{5.f};
			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			//fov---------------------------------------------------------------------------------------------
			if (pKeyboardState[SDL_SCANCODE_UP])
			{
				ChangeFovAngle(fovAngleChange);
			}
			else if (pKeyboardState[SDL_SCANCODE_DOWN])
			{
				ChangeFovAngle(-fovAngleChange);
			}
			//movement-------------------------------------------------------------------------------------------
			float movementSpeed{ 25.f };
			float angularMovementSpeed{ 180.f * TO_RADIANS };

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * movementSpeed * deltaTime;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//hold and drag to move
			if ((mouseState & SDL_BUTTON_RMASK) && (mouseState & SDL_BUTTON_LMASK))
			{
				if (mouseY > 0)
				{
					origin += -up * movementSpeed * deltaTime;
				}
				if (mouseY < 0)
				{
					origin += up * movementSpeed * deltaTime;
				}
				if (mouseX > 0)
				{
					origin += right * movementSpeed * deltaTime;
				}
				if (mouseX < 0)
				{
					origin += -right * movementSpeed * deltaTime;
				}
			}
			//lmb movement?
			else if (mouseState & SDL_BUTTON_LMASK)
			{
				if (mouseY > 0)
				{
					origin += forward * -movementSpeed * deltaTime;
				}
				if (mouseY < 0)
				{
					origin += forward * movementSpeed * deltaTime;
				}
				if (mouseX > 0)
				{
					totalYaw += angularMovementSpeed * deltaTime;
				}
				if (mouseX < 0)
				{
					totalYaw -= angularMovementSpeed * deltaTime;
				}
				const Matrix totalRotationMatrix{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };
				forward = totalRotationMatrix.TransformVector(Vector3::UnitZ);
				forward.Normalize();
				right = totalRotationMatrix.TransformVector(Vector3::UnitX);
				right.Normalize();

			}
			//rmb rotation
			else if (mouseState & SDL_BUTTON_RMASK)
			{
				if (mouseX > 0)
				{
					totalYaw += angularMovementSpeed * deltaTime;
				}
				if (mouseX < 0)
				{
					totalYaw -= angularMovementSpeed * deltaTime;
				}
				if (mouseY > 0)
				{
					totalPitch -= angularMovementSpeed * deltaTime;
				}
				if (mouseY < 0)
				{
					totalPitch += angularMovementSpeed * deltaTime;
				}

				const Matrix totalRotationMatrix{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };
				forward = totalRotationMatrix.TransformVector(Vector3::UnitZ);
				forward.Normalize();
				right = totalRotationMatrix.TransformVector(Vector3::UnitX);
				right.Normalize();
			}

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
