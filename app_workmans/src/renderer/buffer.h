#pragma once

namespace AppWMs {

	enum class ShaderDataType {
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::Float: return 4;
		case ShaderDataType::Float2: return 4*2;
		case ShaderDataType::Float3: return 4*3;
		case ShaderDataType::Float4: return 4*4;
		case ShaderDataType::Mat3: return 4*3*3;
		case ShaderDataType::Mat4: return 4*4*4;
		case ShaderDataType::Int: return 4;
		case ShaderDataType::Int2: return 4*2;
		case ShaderDataType::Int3: return 4*3;
		case ShaderDataType::Int4: return 4*4;
		case ShaderDataType::Bool: return 1;
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown data type !")
		return 0;
	}

	struct BufferElements {
		ShaderDataType shaderDataType;
		std::string name;
		uint32_t size;
		uint32_t offset;
		bool normalized;

		uint32_t getComponentCount() const{
			switch (shaderDataType) {
			case ShaderDataType::Float: return 1;
			case ShaderDataType::Float2: return 2;
			case ShaderDataType::Float3: return 3;
			case ShaderDataType::Float4: return 4;
			case ShaderDataType::Mat3: return 3 * 3;
			case ShaderDataType::Mat4: return 4 * 4;
			case ShaderDataType::Int: return 1;
			case ShaderDataType::Int2: return 2;
			case ShaderDataType::Int3: return 3;
			case ShaderDataType::Int4: return 4;
			case ShaderDataType::Bool: return 1;
			}
			APP_WMS_CORE_ASSERTS(false, "No count! !")
			return 0;
		}
		BufferElements(ShaderDataType type, const std::string& name, bool normalized =false)
			:shaderDataType(type), name(name), 
			size(ShaderDataTypeSize(type)), offset(0), normalized(normalized){ }
	};

	class BufferLayout {
	private:
		std::vector<BufferElements> pm_bufferElements;
		uint32_t pm_stride=0;

		void calculateOffsetAndStride() {
			uint32_t offset = 0;
			pm_stride = 0;
			for (auto& e : pm_bufferElements) {
				e.offset = offset;
				offset += e.size;
				pm_stride += e.size;
			}

		};
	public:
		BufferLayout();
		BufferLayout(const std::initializer_list<BufferElements>& elements):pm_bufferElements(elements){
			calculateOffsetAndStride();
		}
		inline const std::vector<BufferElements>& getElements() const { return pm_bufferElements; }
		inline uint32_t getStride() const { return pm_stride; }

		// allow to do : for(const BufferElements& element : BufferLayout's instance object)
		std::vector<BufferElements>::iterator begin() { return pm_bufferElements.begin(); }
		std::vector<BufferElements>::iterator end() { return pm_bufferElements.end(); }
		std::vector<BufferElements>::const_iterator begin() const { return pm_bufferElements.begin(); }
		std::vector<BufferElements>::const_iterator end() const { return pm_bufferElements.end(); }

	};

	class VertexBuffer {
	public:
		~VertexBuffer();
		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual const BufferLayout& getLayout() const = 0;
		// layout will be passing through OpenGLVertexArray::addVertexBuffer
		virtual void setLayout(const BufferLayout& layout) = 0;
		virtual void setData(const void* VB, uint32_t size) = 0;


		static Ref<VertexBuffer> create(float* vertices, uint32_t size);
		static Ref<VertexBuffer> create( uint32_t size);

	};

	class IndexBuffer {
	public:
		~IndexBuffer();
		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual uint32_t getCount() const = 0;

		static Ref<IndexBuffer> create(uint32_t* indexes, uint32_t count);
	};

}