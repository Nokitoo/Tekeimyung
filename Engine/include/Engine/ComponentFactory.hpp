/**
* @Author   Guillaume Labey
*/

#pragma once

#include <ImGuizmo.h>
#include <utility>
#include <algorithm>

#include <ECS/Entity.hpp>

#include <Engine/Utils/Exception.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Components.hh>
#include <Engine/Utils/JsonValue.hpp>

// Generate map initializer lists
// ex: { "sRenderComponent", sRenderComponent }, { "sPositionComponent", sPositionComponent }
#define COMPONENTS_TYPES(PROCESS)\
    PROCESS(sRenderComponent),\
    PROCESS(sPositionComponent),\
    PROCESS(sDirectionComponent),\
    PROCESS(sBoxColliderComponent),\
    PROCESS(sSphereColliderComponent),\
    PROCESS(sGravityComponent),\
    PROCESS(sTypeComponent),\
    PROCESS(sParticleEmitterComponent),\
    PROCESS(sNameComponent),\
    PROCESS(sTransformComponent),\
    PROCESS(sResolutionComponent),\
    PROCESS(sButtonComponent),\
    PROCESS(sTileComponent),\
    PROCESS(sScriptComponent)\

#define GENERATE_PAIRS(COMPONENT) { #COMPONENT, new ComponentFactory<COMPONENT>() }

#define GENERATE_PAIRS_HASHS(COMPONENT) { COMPONENT::identifier, #COMPONENT }


class IParamAnimation;

/*
** IComponentFactory
*/

class IComponentFactory
{
public:
    IComponentFactory() {}
    virtual ~IComponentFactory() {}

    // Json load/save
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json) = 0;
    // The component to save can be pass to saveToJson, else it will save the entity template component in _components[entityType]
    // The destination json can also be pass to saveToJson, else it will save the entity template component json in _componentsJson[entityType]
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr) = 0;
    virtual void save(const std::string& entityType, sComponent* component) = 0;
    virtual void remove(const std::string& entityType) = 0;

    // IComponentFactory methods
    static bool                                                     componentTypeExists(const std::string& type);
    static void                                                     initComponent(const std::string& entityType, const std::string& name, const JsonValue& value);
    static sComponent*                                              createComponent(const std::string& entityType, const std::string& name);
    static IComponentFactory*                                       getFactory(const std::string& name);

    static std::string                                              getComponentNameWithHash(uint32_t hash);
    static std::size_t                                              getComponentHashWithName(const std::string& name);
    static const std::unordered_map<uint32_t, std::string>&         getComponentsTypesHashs();

    // ComponentFactory overloaded classes methods
    // Ex: ComponentFactory<sPositionComponent>, ComponentFactory<sInputComponent>
    virtual sComponent*                                             clone(const std::string& entityType) = 0;
    virtual void                                                    addComponent(const std::string& entityType, sComponent* component) = 0;
    virtual void                                                    saveComponentJson(const std::string& entityType, const JsonValue& json) = 0;
    virtual bool                                                    updateEditor(const std::string& entityType, sComponent** savedComponent, sComponent* entityComponent, Entity* entity) = 0;
private:
    // Store Components types
    static std::unordered_map<std::string, IComponentFactory*>      _componentsTypes;

    // Map lookup to get component name with component hash
    static std::unordered_map<uint32_t, std::string>             _componentsTypesHashs;
};


/*
** BaseComponentFactory
*/

template <typename T>
class BaseComponentFactory: public IComponentFactory
{
public:
    BaseComponentFactory()
    {
        #define GET_COMPONENT_NAME(component)   \
            {                                   \
                _name = #component;             \
            }
        GET_COMPONENT_NAME(T)
        #undef GET_COMPONENT_NAME
    }
    virtual ~BaseComponentFactory() {}

    // loadFromJson has to be overloaded in BaseComponentFactory child classes
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json)
    {
        EXCEPT(NotImplementedException, "Failed to load component: the component has no overload to load from json");
    }

    // A component can't be saved without this overload
    // In case it's not defined, the loaded component json will be saved
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr)
    {
        LOG_WARN("%s::%s: : can't save to json because no saveToJson found. The previously loaded json will be saved", entityType.c_str(), _name.c_str());
        return _componentsJson[entityType];
    }

    virtual void save(const std::string& entityType, sComponent* component)
    {
        delete _components[entityType];
        _components[entityType] = component->clone();
    }

    virtual void remove(const std::string& entityType)
    {
        _components.erase(entityType);
    }

    // Add entity in component entities map
    virtual void addComponent(const std::string& entityType, sComponent* component)
    {
        _components[entityType] = component;
    }

    // Save component json in case saveToJson is not overloaded and
    // we need to return the default json
    virtual void    saveComponentJson(const std::string& entityType, const JsonValue& json)
    {
        _componentsJson[entityType] = json;
    }

    // Overload this function to display the component editor
    // The component_ pointer have to be set to component pointer (_components[entityType] = component)
    virtual bool    updateEditor(const std::string& entityType, sComponent** savedComponent, sComponent* entityComponent, Entity* entity)
    {
        return (false);
    }

private:
    virtual sComponent*         clone(const std::string& entityType)
    {
        return _components[entityType]->clone();
    }

protected:
    // One component per entity type
    std::unordered_map<std::string, sComponent*>    _components;

    // Store component JSON retrieved from loadFromJson function
    // so we can return the default json if saveToJson is not implemented
    std::unordered_map<std::string, JsonValue>    _componentsJson;

private:
    // Component name
    std::string                                     _name;
};


/*
** ComponentFactory
*/
template <typename T>
class ComponentFactory
{
public:
    ComponentFactory() {}
    ~ComponentFactory() {}
};

/*
** sRenderComponent
*/

template <>
class ComponentFactory<sRenderComponent>: public BaseComponentFactory<sRenderComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);

private:
    void            loadTranslateParamAnimation(std::shared_ptr<ParamAnimation<glm::vec3>> paramAnimation, JsonValue& json);
    void            loadColorParamAnimation(std::shared_ptr<ParamAnimation<glm::vec4>> paramAnimation, JsonValue& json);

    void            saveTranslateParamAnimation(std::shared_ptr<IParamAnimation> paramAnimation_, JsonValue& json);
    void            saveColorParamAnimation(std::shared_ptr<IParamAnimation> paramAnimation_, JsonValue& json);

    virtual bool    updateEditor(const std::string& entityType, sComponent** savedComponent, sComponent* entityComponent, Entity* entity);
    bool            updateAnimationsEditor(sRenderComponent* component, Entity* entity);
    bool            updateParamsAnimationsEditor(AnimationPtr playedAnimation, Entity* entity);
    bool            updateAnimationParamTranslate(Entity* entity, std::shared_ptr<IParamAnimation> paramAnimation_, uint32_t& frameNb);
    bool            updateAnimationParamColor(std::shared_ptr<IParamAnimation> paramAnimation_, uint32_t& frameNb);
};


/*
** sPositionComponent
*/

template <>
class ComponentFactory<sPositionComponent>: public BaseComponentFactory<sPositionComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);
};

/*
** sDirectionComponent
*/

template <>
class ComponentFactory<sDirectionComponent>: public BaseComponentFactory<sDirectionComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);
};


/*
** sBoxColliderComponent
*/

template <>
class ComponentFactory<sBoxColliderComponent>: public BaseComponentFactory<sBoxColliderComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);

    virtual bool    updateEditor(const std::string& entityType, sComponent** savedComponent, sComponent* entityComponent, Entity* entity);
};


/*
** sSphereColliderComponent
*/

template <>
class ComponentFactory<sSphereColliderComponent>: public BaseComponentFactory<sSphereColliderComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);

    virtual bool    updateEditor(const std::string& entityType, sComponent** savedComponent, sComponent* entityComponent, Entity* entity);
};

/*
** sResolutionComponent
*/
template <>
class ComponentFactory<sResolutionComponent> : public BaseComponentFactory<sResolutionComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
};

/*
** sGravityComponent
*/

template <>
class ComponentFactory<sGravityComponent>: public BaseComponentFactory<sGravityComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);
};


/*
** sTypeComponent
*/

template <>
class ComponentFactory<sTypeComponent>: public BaseComponentFactory<sTypeComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);
    virtual eEntityType stringToEntityType(const std::string& entityTypeStr);
    virtual std::string entityTypeToString(eEntityType entityType);
};

/*
** sParticleEmitterComponent
*/

template <>
class ComponentFactory<sParticleEmitterComponent>: public BaseComponentFactory<sParticleEmitterComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);

    virtual bool    updateEditor(const std::string& entityType, sComponent** savedComponent, sComponent* entityComponent, Entity* entity);
};

/*
** sNameComponent
*/

template <>
class ComponentFactory<sNameComponent> : public BaseComponentFactory<sNameComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue& saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);
};


/*
** sTransformComponent
*/

template <>
class ComponentFactory<sTransformComponent> : public BaseComponentFactory<sTransformComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
    virtual JsonValue&  saveToJson(const std::string& entityType, const sComponent* savedComponent = nullptr, JsonValue* toJson = nullptr);

    virtual bool    updateEditor(const std::string& entityType, sComponent** savedComponent, sComponent* entityComponent, Entity* entity);

    // Update transforms with gizmos
    // Used for multiple components: sTransformComponent, sBoxColliderComponent
    static bool     updateTransforms(glm::vec3& pos, glm::vec3& scale, glm::vec3& rotation, glm::vec3& savedRotation, glm::mat4& transform, ImGuizmo::MODE mode);
};


/*
** sButtonComponent
*/

template <>
class ComponentFactory<sButtonComponent> : public BaseComponentFactory<sButtonComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
};


/*
** sTileComponent
*/

template <>
class ComponentFactory<sTileComponent> : public BaseComponentFactory<sTileComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
};


/*
** sScriptComponent
*/

template <>
class ComponentFactory<sScriptComponent> : public BaseComponentFactory<sScriptComponent>
{
public:
    virtual sComponent* loadFromJson(const std::string& entityType, const JsonValue& json);
};
