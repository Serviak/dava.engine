#pragma once

#include "Entity/SceneSystem.h"
#include "FileSystem/FilePath.h"
#include "Base/RefPtr.h"
#include "Base/FastName.h"

#include <memory>

namespace DAVA
{
class SlotComponent;

/**
    \ingroup systems
    \brief SlotSystem manage slots (SlotComponent), loaded items and provide methods to load item into slot or into all
    slots with some name. SlotSystem delegate item loading to ExternalEntityLoader, that can implement some custom logic.
    For example, hold cache of Entities or add some properties to loaded Entity. By default ExternalEntityLoader implement
    async loaded strategy. Method AttachItemToSlot will return 'empty Entity' immediately. Loading will process in separate
    thread. Item will be attached into 'empty Entity' after it will be fully loaded.

    SlotSystem use ItemsCache to hold result of config files parsing. By default SlotSystem use one ItemsCache per scene,
    but game can override this behaviour and call SetSharedCache to change scope of this cache.

    Config parsing is a lazy operation. In method AttachItemToSlot SlotSystem call ItemsCache to find item with some name.
    If cache has not parse config file of SlotComponent yet, it will parse this config.
*/
class SlotSystem final : public SceneSystem
{
public:
    class ItemsCache
    {
    public:
        struct Item
        {
            FastName itemName;
            FastName type;
            FilePath scenePath;
            RefPtr<KeyedArchive> additionalParams;
        };

        void LoadConfigFile(const FilePath& configPath);
        const Item* LookUpItem(const FilePath& configPath, FastName itemName);
        Vector<Item> GetItems(const FilePath& configPath);

    private:
        void LoadYamlConfig(const FilePath& configPath);
        void LoadXmlConfig(const FilePath& configPath);
        class XmlConfigParser;
        struct ItemLess
        {
            bool operator()(const Item& item1, const Item& item2) const;
        };

        UnorderedMap<String, Set<Item, ItemLess>> cachedItems;
    };

    class ExternalEntityLoader
    {
    public:
        virtual Entity* Load(const FilePath& path) = 0;
        virtual void AddEntity(Entity* parent, Entity* child) = 0;
        virtual void Process(float32 delta) = 0;

        void SetScene(Scene* scene);

    protected:
        Scene* scene = 0;
    };

    SlotSystem(Scene* scene);
    ~SlotSystem();

    /**
        Override default ItemsCache. Changing of items cache will produce cache invalidation,
        but will not produce unloading of loaded items 
    */
    void SetSharedCache(std::shared_ptr<ItemsCache> cache);
    /**
        Get result of parsing config file. This method can be used to precache config files
        \return Items that described in \c configPath
    */
    Vector<ItemsCache::Item> GetItems(const FilePath& configPath);
    /** Override default async loader */
    void SetExternalEntityLoader(std::shared_ptr<ExternalEntityLoader> externalEntityLoader);

    void UnregisterEntity(Entity* entity) override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;

    void Process(float32 timeElapsed) override;

    /**
        Load and attach item with name \c itemName into all slots with name \c slotName recursively in hierarchy with root \c rootEntity
        \arg \c rootEntity root of hierarchy. Should be attached to scene
    */
    void AttachItemToSlot(Entity* rootEntity, FastName slotName, FastName itemName);
    /**
        Load and attach item with name \c itemName into slots \c component
        \return Loaded and attached to slot Entity
    */
    Entity* AttachItemToSlot(SlotComponent* component, FastName itemName);
    /** Attach \c entity into slot \c component and named it by name equal to \c itemName */
    void AttachEntityToSlot(SlotComponent* component, Entity* entity, FastName itemName);

    /** Lookup entity that currently loaded into slot \c component. Can return nullptr if slot currently empty */
    Entity* LookUpLoadedEntity(SlotComponent* component) const;
    /** Lookup slot by \c entity that currently loaded into it. Can return nullptr if \c entity is not loaded item*/
    SlotComponent* LookUpSlot(Entity* entity) const;
    /** Helper function to get local transform of joint that \c component attached to. If \c component doesn't attached to joint, return Identity*/
    Matrix4 GetJointTransform(SlotComponent* component) const;

protected:
    void SetScene(Scene* scene) override;

private:
    void UnloadItem(SlotComponent* component);

    UnorderedMap<Component*, Entity*> slotToLoadedEntity;
    UnorderedMap<Entity*, Component*> loadedEntityToSlot;

    std::shared_ptr<ExternalEntityLoader> externalEntityLoader;
    std::shared_ptr<ItemsCache> sharedCache;

    Vector<Entity*> deletePending;
};

} // namespace DAVA
