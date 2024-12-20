#ifndef ENTITY_H
#define ENTITY_H

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"
#include "../algorithm/cstr_map.h"
#include "iostream"
#include "keyvalue.h"

namespace gigno {
	class Application;

	/*
		ENTITY:
	Base class for any dynamic object in the scene, that needs to be updated regularly.
	To Update themselves, entity may override any of those methods :
	-	void Think : Called every frame, the duration of said frame is passed as argument.
	-	void PhysicThink : 	Called every physic tick, physic ticks are separated by a regular interval, 
							value of which is passed as argument.
	- 	void LatePhysicthink: Called after every entity are done with their PhysicThink.

	Every entities also have transformation infos : Position, Rotation and Scale.

	Every entity must be default-constructible and have a constructor that does not depend on the app
	Meaning : if you need to use GetApp()->Function(), wrap it in a if(GetApp()) block !

	AN entity may only derive from ONE entity.

	To interface with the current application, use the method GetApp().

	Evey entity declarations require all of the following macros :
		At the top of the class declaration, use the macro ENTITY_DECLARATIONS() (that declares reqired methods)
		In a cpp file, use macro ENTITY_DEFINITIONS() (that defines the method we declared)
		Under the class declaration, use macro BEGIN_KEY_TABLE() and END_KEY_TABLE.

	Keyvalues allows the engine to interface with the properties of your entity to, for example, 
	show these values in the ENtity Inspector or load entities from a scene file (comming soon).

	For any value for which you wish those features to be active, you must tell it using the macro
	DEFINE_KEY_VALUE() in betweemn BEGIN_KEY_TABLE and END_KEY_TABLE, passing in the type of the value and its name
	The value MUST be public (at least right now)

	Even if you dont want to add any keyvalues, you MUST still add the macros BEGIN_KEY_TABLE() and END_KEY_TABLE.
	In that case, you may leave them empty.

	Some types may be unsupported. THe list of supported type is defined in file 'keyvalue.h'.
	*/
	class Entity {
		friend class AddBuildEntityMethod;
	public:
		Entity(const Entity &) = delete;
		Entity & operator=(const Entity &) = delete;
		

		Entity();
		~Entity();

		virtual void Init() {};
		// Called Every Tick by the Entity Server
		virtual void Think(float dt);
		// Called Every physic Tick (fixed time interval)
		virtual void PhysicThink(float dt) {};
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

		
		// Returns a vector of all the key values of this Entity and all its bases.
		virtual std::vector<std::pair<const char *, Value_t>> KeyValues();
		// Returns the number of key values of this entity and all its bases
		virtual size_t KeyValueCount() const;
		virtual Value_t GetKeyValue_t(const char *key);
		// Converts the valueAsString and sets the keyvalue at key to is.
		virtual void SetKeyValue(const char *key, const char **args, int argC);
		// Returns the keyvalue converted to this type. If it doesnt convert, return a default T object.
		template<typename T>
		T GetKeyValue(const char *key) {
			return FromValue<T>(GetKeyValue_t(key));
		}

		virtual const char *GetTypeName() const { return "Entity"; };

		//Creates an entity of the given type. You take ownership of said entity.
		static Entity* CreateEntity(const char *entityType);

		typedef Entity *(*BuildEntityFunc)(void);

	protected:
		Application *GetApp() const;
	private:
		// Matches every entity type name to the "BuildEntity" func that builds said entity.
		inline static CstrUnorderedMap_t<BuildEntityFunc> s_BuildEntityMap{};
	};

	BEGIN_KEY_TABLE(Entity)
		DEFINE_KEY_VALUE(vec3, Position)
		DEFINE_KEY_VALUE(vec3, Rotation)
		DEFINE_KEY_VALUE(vec3, Scale)
		DEFINE_KEY_VALUE(cstr, Name)
	END_KEY_TABLE

	template<typename TEntity>
	inline Entity *BuildEntity();

	// To add values to s_BuildEntityMap from global scope through the constructor.
	class AddBuildEntityMethod {
	public:
		template<class Fn>
		AddBuildEntityMethod(const char *name, Fn &&method) {
			Entity::s_BuildEntityMap[name] = method;
		}
	};


//---------------------------------------------------------------------------------------------------
// ENTITY MACROS
//---------------------------------------------------------------------------------------------------

#define ENTITY_DECLARATIONS(this_class, base_class)                             \
public:                                                                         \
	virtual std::vector<std::pair<const char *, Value_t>> KeyValues() override; \
	virtual size_t KeyValueCount() const override;                              \
	virtual Value_t GetKeyValue_t(const char *key) override;              \
	virtual void SetKeyValue(const char *key, const char **args, int argsC) override;\
	virtual const char *GetTypeName() const override;

#define ENTITY_DEFINITIONS(this_class, base_class)                                                     \
	std::vector<std::pair<const char *, Value_t>> this_class::KeyValues()                              \
	{                                                                                                  \
		std::vector<std::pair<const char *, Value_t>> ret{this_class::KeyValueCount()};                \
		int i = 0;                                                                                     \
		for (auto &[key, owned_value] : KeyTableAccessor<this_class>::KeyValues)                       \
		{                                                                                              \
			ret[i++] = {key, FromOwnedValue(this, owned_value)};                                       \
		}                                                                                              \
		std::vector<std::pair<const char *, Value_t>> base = base_class::KeyValues();                  \
		for (auto &v : base)                                                                           \
		{                                                                                              \
			ret[i++] = v;                                                                              \
		}                                                                                              \
		return ret;                                                                                    \
	}                                                                                                  \
	Value_t this_class::GetKeyValue_t(const char *key)                                                 \
	{                                                                                                  \
		Value_t ret{};                                                                                 \
		if (GetKeyvalue<this_class>(ret, this, key))                                                   \
		{                                                                                              \
			return ret;                                                                                \
		}                                                                                              \
		else                                                                                           \
		{                                                                                              \
			return base_class::GetKeyValue_t(key);                                                     \
		}                                                                                              \
	}                                                                                                  \
	void this_class::SetKeyValue(const char *key, const char **args, int argC)                           \
	{                                                                                                  \
		if(!SetKeyvalueFromString<this_class>(this, key, args, argC)) {							   \
			base_class::SetKeyValue(key, args, argC);									   		   \
		}																							   \
	}                                                                                                  \
	size_t this_class::KeyValueCount() const                                                           \
	{                                                                                                  \
		return base_class::KeyValueCount() + KeyTableAccessor<this_class>::KeyValues.size();           \
	}                                                                                                  \
	const char *this_class::GetTypeName() const { return #this_class; }                                \
	template <>                                                                                        \
	inline Entity *BuildEntity<this_class>() { return new this_class(); };                             \
	namespace                                                                                          \
	{                                                                                                  \
		AddBuildEntityMethod AddBuildEntityMethod_##this_class(#this_class, &BuildEntity<this_class>); \
	}
}

#endif
