#pragma once

#include "Entity/SceneSystem.h"

namespace DAVA
{
class SlotComponent;
class SlotSystem : public SceneSystem
{
public:
    class ItemsCache : public BaseObject
    {
    public:
        struct Item
        {
            FastName itemName;
            FastName tag;
            FilePath scenePath;
            RefPtr<KeyedArchive> additionalParams;
        };

        void LoadConfigFile(const FilePath& configPath);
        const Item* LookUpItem(const FilePath& configPath, const FastName& itemName, const FastName& tag);

    private:
        void LoadYamlConfig(const FilePath& configPath);
        void LoadXmlConfig(const FilePath& configPath);
        struct ItemLess
        {
            bool operator()(const Item& item1, const Item& item2) const;
        };

        UnorderedMap<String, Set<Item, ItemLess>> cachedItems;
    };

    class ExternalEntityLoader : public BaseObject
    {
    public:
        virtual Entity* Load(const FilePath& path) = 0;
        virtual void AddEntity(Entity* parent, Entity* child) = 0;

        void SetScene(Scene* scene);

    protected:
        Scene* scene = 0;
    };

    SlotSystem(Scene* scene);
    ~SlotSystem();

    void SetSharedCache(RefPtr<ItemsCache> cache);
    void SetExternalEntityLoader(RefPtr<ExternalEntityLoader> externalEntityLoader);

    void UnregisterEntity(Entity* entity) override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;

    void Process(float32 timeElapsed) override;

    void AttachItemToSlot(Entity* rootEntity, FastName slotName, FastName itemName);
    void AttachItemToSlot(SlotComponent* component, FastName itemName);
    void AttachEntityToSlot(SlotComponent* component, Entity* entity);

    Entity* LookUpLoadedEntity(SlotComponent* component);

protected:
    void SetScene(Scene* scene) override;
    void UnloadItem(SlotComponent* component);

private:
    UnorderedMap<Component*, Entity*> slotToLoadedEntity;
    UnorderedMap<Entity*, Component*> loadedEntityToSlot;

    RefPtr<ExternalEntityLoader> externalEntityLoader;
    RefPtr<ItemsCache> sharedCache;

    Vector<Entity*> deletePending;
};

} // namespace DAVA