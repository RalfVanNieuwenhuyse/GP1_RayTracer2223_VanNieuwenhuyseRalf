#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{			
			const Vector3 sphereOrToRayOr{ ray.origin - sphere.origin };
			const float a{ Vector3::Dot(ray.direction, ray.direction) };
			const float b{ 2 * Vector3::Dot(ray.direction, sphereOrToRayOr) };
			const float c{ Vector3::Dot(sphereOrToRayOr, sphereOrToRayOr) - sphere.radius * sphere.radius };

			const float discriminant{ b * b - 4 * a * c };

			if (discriminant > 0)
			{
				const float sqrtDiscriminant{ sqrtf(discriminant) };

				const float t0{ (-b - sqrtDiscriminant) / (2.f * a) };

				if (t0 > ray.min && t0 < ray.max)
				{
					if (ignoreHitRecord) return true;

					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = ray.origin + t0 * ray.direction;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					hitRecord.t = t0;

					return true;
				}

				const float t1{ (-b + sqrtDiscriminant) / (2.f * a) };

				if (t1 > ray.min && t1 < ray.max)
				{
					if (ignoreHitRecord) return true;

					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = ray.origin + t1 * ray.direction;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					hitRecord.t = t1;

					return true;
				}
			}
			//assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1		

			const float t{ Vector3::Dot(plane.origin - ray.origin, plane.normal) / Vector3::Dot(ray.direction, plane.normal) };

			if (t >= ray.min && t <= ray.max)
			{
				if (ignoreHitRecord) return true;

				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.normal = plane.normal;
				hitRecord.origin = ray.origin + t * ray.direction;
				hitRecord.t = t;

				return true;
			}
			
			//assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = true)
		{
			//todo W5
			const Vector3 v{ ray.direction };
			const TriangleCullMode cullMode{ triangle.cullMode };
			const Vector3 TriangleNormal{ triangle.normal };
			const float dotNormalViewRay{ Vector3::Dot(TriangleNormal, v) };

			if (ignoreHitRecord)
			{
				if (cullMode == TriangleCullMode::BackFaceCulling && dotNormalViewRay < 0)
				{
					return false;
				}

				if (cullMode == TriangleCullMode::FrontFaceCulling && dotNormalViewRay > 0)
				{
					return false;
				}
			}
			else
			{
				if (cullMode == TriangleCullMode::BackFaceCulling && dotNormalViewRay > 0) 
				{
					return false;
				}

				if (cullMode == TriangleCullMode::FrontFaceCulling && dotNormalViewRay < 0)
				{
					return false;
				}
			}

			if (dotNormalViewRay == 0.f)
			{
				return false;
			}

			const Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3.f };
			const Vector3 L{ center - ray.origin };
			
			const float t = Vector3::Dot(L, TriangleNormal) / dotNormalViewRay;

			if (t > ray.min && t < ray.max)
			{
				const Vector3 hitPoint{ ray.origin + t * v };

				const Vector3 edgeA{ triangle.v1 - triangle.v0 };
				const Vector3 point0ToHitPoint{ hitPoint - triangle.v0 };
				if (Vector3::Dot(Vector3::Cross(edgeA, point0ToHitPoint), TriangleNormal) < 0)
				{
					return false;
				}

				const Vector3 edgeB{ triangle.v2 - triangle.v1 };
				const Vector3 point1ToHitPoint{ hitPoint - triangle.v1 };
				if (Vector3::Dot(Vector3::Cross(edgeB, point1ToHitPoint), TriangleNormal) < 0)
				{
					return false;
				}

				const Vector3 edgeC{ triangle.v0 - triangle.v2 };
				const Vector3 point2ToHitPoint{ hitPoint - triangle.v2 };
				if (Vector3::Dot(Vector3::Cross(edgeC, point2ToHitPoint), TriangleNormal) < 0)
				{
					return false;
				}

				if (ignoreHitRecord) return true;

				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.normal = triangle.normal;
				hitRecord.origin = ray.origin + t * v;
				hitRecord.t = t;

				return true;
			}


			//assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest

		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			const Vector3 inversedDirection = { 1.f / ray.direction.x,1.f / ray.direction.y,1.f / ray.direction.z };
			const float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) * inversedDirection.x;
			const float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) * inversedDirection.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			const float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) * inversedDirection.y;
			const float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) * inversedDirection.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			const float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) * inversedDirection.z;
			const float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) * inversedDirection.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			//assert(false && "No Implemented Yet!");

			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}

			if (mesh.indices.size() % 3)
			{
				return false;
			}
			
			bool hit{ false };
			HitRecord smallestT;
			smallestT.t = FLT_MAX;
			HitRecord currentRecord;			
			Triangle tri;
			for (size_t i{ 0 }; i < (mesh.indices.size() / 3); ++i)
			{
				const size_t index{ (i * 3) };

				tri = { mesh.transformedPositions[mesh.indices[index]], mesh.transformedPositions[mesh.indices[index + 1]], mesh.transformedPositions[mesh.indices[index + 2]] };
				tri.cullMode = mesh.cullMode;
				tri.materialIndex = mesh.materialIndex;
				tri.normal = mesh.transformedNormals[i];
				if (HitTest_Triangle(tri, ray, currentRecord, ignoreHitRecord))
				{
					if (currentRecord.t < smallestT.t)
					{
						smallestT = currentRecord;
					}
					hit = true;
				}
			}
			hitRecord = smallestT;
			return hit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			//assert(false && "No Implemented Yet!");
			return { light.origin - origin };
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			//assert(false && "No Implemented Yet!");
			if (light.type == LightType::Point)
			{					
				return{ (light.color * light.intensity) / GetDirectionToLight(light,target).SqrMagnitude() };
			}
			if (light.type == LightType::Directional)
			{
				return{ light.color * light.intensity };
			}

			//return ColorRGB{ 0.f, 0.f, 0.f };
			return {};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}