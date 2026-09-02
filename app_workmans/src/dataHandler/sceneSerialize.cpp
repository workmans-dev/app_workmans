#include "pch.h"
#include "sceneSerialize.h"
#include "yaml-cpp/yaml.h"
#include "renderer/camera/cameraController.h"

namespace YAML {
	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (node.size() != 3) {
				return false; // Expecting a node with 4 elements
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};
	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (node.size() != 4) {
				return false; // Expecting a node with 4 elements
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace AppWMs {
	static void load_camComp_data_fromFile(const YAML::Node& camComp_data, CameraMatrixComponent& camComp)
	{
		if (!camComp_data.IsNull()) {
			const bool& defaultCam = camComp_data["DefaultCam"].as<bool>();
			const int& camType = camComp_data["CameraType"].as<int>();
			camComp.sceneCam.set_camProjType((SceneCamera::CameraProjType)camType);
			const float& nearClip = camComp_data["NearClip"].as<float>();
			const float& farClip = camComp_data["FarClip"].as<float>();
			if (camComp_data["CameraType"].as<int>() == 0) {
				// perspective
				const float& FOV = camComp_data["VecticalFOV"].as<float>();
				camComp.sceneCam.set_persp(FOV, nearClip, farClip);
			}
			else {
				// orthographic
				const float& size = camComp_data["Size"].as<float>();
				camComp.sceneCam.set_orthographic(size, nearClip, farClip);
			}
			camComp.primary = camComp_data["Primary"].as<bool>();;
			camComp.fixed_aspectRatio = camComp_data["FixedAspectRatio"].as<bool>();;
			camComp.defaultCam = camComp_data["DefaultCam"].as<bool>();;
		}
	}

	// 트랜스폼 로드. 두 경로(기존 기본 카메라 재사용 / 새 엔티티 생성)가 같은 규칙을
	// 써야 해서 함수로 뺐다 — 한쪽만 고치면 저장은 되는데 로드가 반만 되는 버그가 난다.
	static void load_transform_data_fromFile(const YAML::Node& transComp_data, TransformComponent& transComp)
	{
		transComp.translate = transComp_data["Translate"].as<glm::vec3>();
		transComp.scale = transComp_data["Scale"].as<glm::vec3>();

		// ⚠️ 모드를 값보다 **먼저** 세운다. setRotationMode는 "회전을 유지하며 표현만
		//    바꾸는" 함수라, 값을 넣은 뒤에 부르면 방금 읽은 값을 변환해 덮어쓴다.
		//    먼저 부르면 그 변환 결과는 바로 아래 대입이 전부 덮으므로 무해하다.
		//    옛 .wmsa엔 이 키가 없다 → 기본값(XYZ Euler) 유지 = 하위 호환.
		if (transComp_data["RotationMode"])
			transComp.setRotationMode((RotationMode)transComp_data["RotationMode"].as<int>());

		transComp.rotate = transComp_data["Rotate"].as<glm::vec3>();

		if (transComp_data["RotateQuat"]) {
			// 파일엔 [w, x, y, z] 순으로 적혀 있다. glm::quat 생성자도 (w,x,y,z)다.
			// (메모리 배치는 (x,y,z,w)라 캐스팅으로 넘기면 뒤집힌다)
			const glm::vec4 packed = transComp_data["RotateQuat"].as<glm::vec4>();
			transComp.rotateQuat = glm::quat(packed[0], packed[1], packed[2], packed[3]);
		}
	}

	YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec3& vec) {
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec4& color) {
		out << YAML::Flow;
		out << YAML::BeginSeq << color.x << color.y << color.z << color.w << YAML::EndSeq;
		return out;
	}

	template<typename T, typename BeginMap>
	static void serializeComponent(YAML::Emitter& out, Entity& entity, BeginMap insert) {
		if (entity.hasComponent<T>()) {
			auto& comp = entity.getComponent<T>();
			// 타입 이름은 컴파일 타임 상수다. 예전엔 컴포넌트마다 std::string으로
			// 들고 있었는데, ECS 배열에 인스턴스당 32바이트(+ 힙)를 얹는 낭비였다.
			// 방출되는 문자열은 동일하므로 .wmsa 포맷은 그대로다.
			out << YAML::Key << T::TypeName() << YAML::BeginMap;
			insert(comp);
			//out << YAML::Key << keyName << YAML::Value << tagComp.name;
			out << YAML::EndMap;
		}
	}

	void AppWMs::Serialize::serialize(const std::string& file_path)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		for (auto& entityID : pm_scene->pm_register.view<entt::entity>()) {
			Entity entity = { entityID, pm_scene.get() };
			if (entity.get_entityID()!=entt::null)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Entity" << YAML::Value << (uint32_t)entity.get_entityID();
				serializeComponent<TagComponent>(out, entity, [&out](auto& comp) {
					out << YAML::Key << "Name" << YAML::Value << comp.name;
					out << YAML::Key << "NameEditable" << YAML::Value << comp.nameEditable;
					});
				serializeComponent<TransformComponent>(out, entity, [&out](auto& comp) {
					out << YAML::Key << "Translate" << YAML::Value << comp.translate;
					out << YAML::Key << "Rotate" << YAML::Value << comp.rotate;
					out << YAML::Key << "Scale" << YAML::Value << comp.scale;
					// 회전 모드와 쿼터니언은 새 필드다. 옛 .wmsa엔 없으므로
					// 로드 쪽이 없을 때 기본값(XYZ Euler)으로 떨어지게 해뒀다.
					out << YAML::Key << "RotationMode" << YAML::Value << (int)comp.getRotationMode();
					// ⚠️ vec4로 적되 순서는 (w,x,y,z)다. glm::quat의 메모리 배치는
					//    (x,y,z,w)라 무심코 캐스팅하면 뒤집힌다.
					out << YAML::Key << "RotateQuat" << YAML::Value
						<< glm::vec4(comp.rotateQuat.w, comp.rotateQuat.x,
							comp.rotateQuat.y, comp.rotateQuat.z);
					});
				serializeComponent<CameraMatrixComponent>(out, entity, [&out](auto& comp) {
					const int& camType = (int)comp.sceneCam.get_camProjType();
					out << YAML::Key << "CameraType" << YAML::Value << camType;
					if (camType == 0) {
						out << YAML::Key << "VecticalFOV" << YAML::Value << comp.sceneCam.get_persp_FOV();
						out << YAML::Key << "NearClip" << YAML::Value << comp.sceneCam.get_persp_nearClip();
						out << YAML::Key << "FarClip" << YAML::Value << comp.sceneCam.get_persp_farClip();
					}
					else {
						out << YAML::Key << "Size" << YAML::Value << comp.sceneCam.get_ortho_size();
						out << YAML::Key << "NearClip" << YAML::Value << comp.sceneCam.get_ortho_nearClip();
						out << YAML::Key << "FarClip" << YAML::Value << comp.sceneCam.get_ortho_farClip();
					}
					out << YAML::Key << "Primary" << YAML::Value << comp.primary;
					out << YAML::Key << "FixedAspectRatio" << YAML::Value << comp.fixed_aspectRatio;
					out << YAML::Key << "DefaultCam" << YAML::Value << comp.defaultCam;

					});
				serializeComponent<ShadingComponent>(out, entity, [&out](auto& comp) {
					out << YAML::Key << "Color" << YAML::Value << comp.color;
					});
				serializeComponent<VisComponent>(out, entity, [&out](auto& comp) {
					out << YAML::Key << "Visible" << YAML::Value << comp.visible;

					});
				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream createFile(file_path);
		createFile << out.c_str();
	}

	void AppWMs::Serialize::runtime_serialize(const std::string& file_path)
	{
	}

	bool Serialize::deserialize(const std::string& file_path)
	{
		std::ifstream file(file_path);
		std::stringstream str_stream;
		str_stream << file.rdbuf();
		YAML::Node data = YAML::Load(str_stream.str());
		if (!data["Scene"])
			return false;
		const auto& entities_data = data["Entities"];
		const std::string& sceneName = data["Scene"].as<std::string>();

		// default camera.
		std::unordered_map<std::string, Entity> name_camEntity_map;
		for (auto& cam_entity : pm_scene->pm_cam_entities) {
			name_camEntity_map[cam_entity.getComponent<TagComponent>().name] = cam_entity;
		}
		
		for (auto& entity_data : entities_data) {
			const std::string& camName_data = entity_data["TagComponent"]["Name"].as<std::string>();

			if (name_camEntity_map.find(camName_data) != name_camEntity_map.end()) {
				auto& camComp = name_camEntity_map[camName_data].getComponent<CameraMatrixComponent>();
				auto& transComp = name_camEntity_map[camName_data].getComponent<TransformComponent>();
				
				load_camComp_data_fromFile(entity_data["CameraMatrixComponent"], camComp);
				if (entity_data["TransformComponent"])
					load_transform_data_fromFile(entity_data["TransformComponent"], transComp);
			}
			else {
				// ⚠️ "무슨 종류인가"를 파일에 따로 저장하지 않는다 — 어떤 컴포넌트가
				//    들어 있는지가 그 질문의 답이다. TransformComponent가 있으면
				//    공간에 있는 오브젝트고, 없으면 위치 없는 엔티티(모디파이어 등)다.
				//    종류 필드를 따로 두면 그 필드와 실제 구성이 어긋날 수 있다.
				Entity entity = entity_data["TransformComponent"]
					? pm_scene->createObject(camName_data)
					: pm_scene->createEntity(camName_data);
				if (entity_data["CameraMatrixComponent"]) {
					auto& new_camComp = entity.addComponent<CameraMatrixComponent>();
					//entity.addComponent<NativeScriptComponent>().bind<CameraController>();
					load_camComp_data_fromFile(entity_data["CameraMatrixComponent"], new_camComp);
					pm_scene->pm_cam_entities.emplace_back(entity);
					pm_scene->pm_cameraViewArray.emplace_back(camName_data);
				}
				if (entity_data["TransformComponent"])
					load_transform_data_fromFile(entity_data["TransformComponent"],
						entity.getComponent<TransformComponent>());
				if (entity_data["ShadingComponent"]) {
					auto& shadingComp_data = entity_data["ShadingComponent"];
					entity.addComponent<ShadingComponent>(shadingComp_data["Color"].as<glm::vec4>());
				}
				// VisComponent는 오브젝트에만 붙는다 (createObject가 얹는다).
				if (entity_data["VisComponent"] && entity.hasComponent<VisComponent>()) {
					auto& visComp_data = entity_data["VisComponent"];
					entity.getComponent<VisComponent>().visible = visComp_data["Visible"].as<int>();
				}

			}
		}

		return true;
	}

	bool Serialize::runtime_deserialize(const std::string& file_path)
	{
		return true;
	}




}