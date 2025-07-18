#ifndef ENTITY_H
#define ENTITY_H

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"
#include <string>
#include "iostream"

#include "serialization.h"

namespace gigno {
	class Application;

	struct Transform_t
	{
		glm::vec3 Position{};
		glm::vec3 Scale{1.0f, 1.0f, 1.0f};
		glm::vec3 Rotation{};

		glm::mat4 TransformationMatrix() const;
		glm::mat3 NormalMatrix() const;
	};

	/*
	Base object. Updated every frames by the giEnityServer.
	Meant to be derived by any ojbect that needs to update.

	Usage:

		* Inheriting: 
			* Any class inheriting Entity should have, at the top of their declaration, the following macros:
				``` 
					BEGIN_SERIALIZE( Class, BaseClass )
					SERIALIZE( type, value ) // If you want to serialize data, as many as you want
					END_SERIALIZE()
				```
				----/OR/----
				```
					ENABLE_SERIALIZE( Class, BaseClass )
				```
				See top of the file serialization.h for more info.
			* Any class inheriting Entity that overrides the constructor should call the base constructor.
			* Any class inheriting Entity that overrides Start() should call the base start.
		*Key Functions:
			* Start() called before the main loop begins, after the constructor.
			* Think( ... ) called every frame with the frame time as parameter.
			* GetApp() returns the current app. Should be used if you need a reference to any core App
			  System ( RenderingServer, InputServer, ...)
	
	Implementation:
		* The Entity base class (here), unlike any class inheriting it, defines its serialized data with the base 
		  function definition of AddSerializedProperties() ( See entity.cpp ).
	*/
	class Entity {
		friend class Serialization;
	public:
		Entity(const Entity &) = delete;
		Entity &operator=(const Entity &) = delete;
		Entity(Entity &&) = default;
		Entity &operator=(Entity &&) = default;

		Entity();
		~Entity();

		virtual void Start() { AddSerializedProperties(); };
		// Called Every Tick by the Entity Server
		virtual void Think(float dt) {};

		const char *Name = "";
		Transform_t Transform{};

		// Next entity in the EntityServer's chain of all entities (linked list). Set on construction.
		// 'nullptr' if is last element.
		Entity* pNextEntity;
	protected:
		Application *GetApp() const;

		//SERIALIZATION--------------------------------------------------------------

		/*
		Pointers to heap-allocated serialized properties for every data that we want to serialize
		*/
		std::vector<BaseSerializedProperty *> serializedProps{};
		virtual void AddSerializedProperties();
	public:
		virtual const char *GetTypeName() const { return "Entity"; };
	};

}
#endif
