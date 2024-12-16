#ifndef ENTITY_H
#define ENTITY_H

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"
#include <string>
#include "iostream"
#include "keyvalues/key_table.h"

namespace gigno {
	class Application;

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

		virtual void Start() { /*AddSerializedProperties();*/ };
		// Called Every Tick by the Entity Server
		virtual void Think(float dt);
		// Called Every physic Tick (fixed time interval)
		virtual void PhysicThink(float dt) {};
		// Called Every physic Tick after PhysicThink.
		virtual void LatePhysicThink(float dt) {};

		const char *Name = "";

		glm::vec3 Position{};
		glm::vec3 Rotation{};
		glm::vec3 Scale{1.0f};

		glm::mat4 TransformationMatrix() const;
		glm::mat3 NormalMatrix() const;
		glm::vec3 ApplyRotate(glm::vec3 v) const;

		// Next entity in the EntityServer's chain of all entities (linked list). Set on construction.
		// 'nullptr' if is last element.
		Entity* pNextEntity;

		/*
		Returns a vector of all the key values of this Entity and all its bases.
		*/
		virtual std::vector<std::pair<const char *, Value_t>> KeyValues();
		virtual size_t KeyValueCount() const;

	protected:
		Application *GetApp() const;

	public:
		virtual const char *GetTypeName() const { return "Entity"; };
	};

	BEGIN_KEY_TABLE(Entity)
		DEFINE_KEY_VALUE(vec3, Position)
		DEFINE_KEY_VALUE(vec3, Rotation)
	END_KEY_TABLE

#define ENTITY_DECLARATIONS(this_class, base_class)                             \
public:                                                                         \
	virtual std::vector<std::pair<const char *, Value_t>> KeyValues() override; \
	virtual size_t KeyValueCount() const override;                              \
	virtual const char *GetTypeName() const override;

#define ENTITY_DEFINITIONS(this_class, base_class)                                           \
	std::vector<std::pair<const char *, Value_t>> this_class::KeyValues()                    \
	{                                                                                        \
		std::vector<std::pair<const char *, Value_t>> ret{this_class::KeyValueCount()};      \
		int i = 0;                                                                           \
		for (auto &[key, owned_value] : KeyTableAccessor<this_class>::KeyValues)             \
		{                                                                                    \
			ret[i++] = {key, FromOwnedValue(this, owned_value)};                             \
		}                                                                                    \
		std::vector<std::pair<const char *, Value_t>> base = base_class::KeyValues();        \
		for (auto &v : base)                                                                 \
		{                                                                                    \
			ret[i++] = v;                                                                    \
		}                                                                                    \
		return ret;                                                                          \
	}                                                                                        \
	size_t this_class::KeyValueCount() const                                                 \
	{                                                                                        \
		return base_class::KeyValueCount() + KeyTableAccessor<this_class>::KeyValues.size(); \
	}                                                                                        \
	const char *this_class::GetTypeName() const { return #this_class; }
}

#endif
