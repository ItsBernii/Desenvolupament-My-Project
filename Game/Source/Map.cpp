
#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Physics.h"
#include "Scene.h"
#include "Defs.h"
#include "Log.h"

#include <math.h>
#include "SDL_image/include/SDL_image.h"

Map::Map() : Module(), mapLoaded(false)
{
    name.Create("map");
}

// Destructor
Map::~Map()
{}

// Called before render is available
bool Map::Awake(pugi::xml_node& config)
{
    LOG("Loading Map Parser");
    bool ret = true;
    texturePath = config.child("mouseTileTex").attribute("texturepath").as_string();
    return ret;
}

bool Map::Start() {

    //Calls the functon to load the map, make sure that the filename is assigned
    SString mapPath = path;
    mapPath += name;
    bool ret = Load(mapPath);

    //Initialize pathfinding 
    pathfinding = new PathFinding();
    pathfinding->mouseTileTex = app->tex->Load(texturePath.GetString());

    return ret;
}

bool Map::Update(float dt)
{
    if(mapLoaded == false)
        return false;

    ListItem<MapLayer*>* mapLayerItem;
    mapLayerItem = mapData.maplayers.start;

    while (mapLayerItem != NULL) {

        if (mapLayerItem->data->properties.GetProperty("Draw") != NULL && mapLayerItem->data->properties.GetProperty("Draw")->value &&
            (mapLayerItem->data->properties.GetProperty("Parallax") == NULL || mapLayerItem->data->properties.GetProperty("Parallax")->value == false)) {

            iPoint playerPos = app->scene->GetPLayer()->position;
            int xLimitToLeft = MAX(playerPos.x / 23 - 23, 0);
            int xLimitToRight = MIN(playerPos.x / 23 + 23, mapLayerItem->data->width);

            int yLimitTop = MAX(playerPos.y / 23 - 23, 0);
            int yLimitBottom = MIN(playerPos.y / 23 + 23, mapLayerItem->data->height);

            for (int x = xLimitToLeft; x < xLimitToRight; x++)
            {
                for (int y = yLimitTop; y < yLimitBottom; y++)
                {
                    int gid = mapLayerItem->data->Get(x, y);
                    TileSet* tileset = GetTilesetFromTileId(gid);

                    SDL_Rect r = tileset->GetTileRect(gid);
                    iPoint pos = MapToWorld(x, y);

                    app->render->DrawTexture(tileset->texture,
                        pos.x,
                        pos.y,
                        SDL_FLIP_NONE,
                        &r,
                        mapLayerItem->data->parallaxFactor);
                }
            }

           
        }


        if (mapLayerItem->data->properties.GetProperty("Parallax") != NULL && mapLayerItem->data->properties.GetProperty("Parallax")->value) {
            
            for (int x = 0; x < mapLayerItem->data->width; x++)
            {
                for (int y = 0; y < mapLayerItem->data->height; y++)
                {
                    int gid = mapLayerItem->data->Get(x, y);
                    TileSet* tileset = GetTilesetFromTileId(gid);

                    SDL_Rect r = tileset->GetTileRect(gid);
                    iPoint pos = MapToWorld(x, y);

                    app->render->DrawTexture(tileset->texture,
                        pos.x,
                        pos.y,
                        SDL_FLIP_NONE,
                        &r,
                        mapLayerItem->data->parallaxFactor);
                }
            }

        }
         
        mapLayerItem = mapLayerItem->next;

    }

    return true;
}

iPoint Map::MapToWorld(int x, int y) const
{
    iPoint ret;

    // L09: DONE 3: Get the screen coordinates of tile positions for isometric maps 
    if (mapData.type == MapTypes::MAPTYPE_ORTHOGONAL) {
        ret.x = x * mapData.tileWidth;
        ret.y = y * mapData.tileHeight;
    }

    if (mapData.type == MapTypes::MAPTYPE_ISOMETRIC) {
        ret.x = x * mapData.tileWidth / 2 - y * mapData.tileWidth / 2;
        ret.y = x * mapData.tileHeight / 2 + y * mapData.tileHeight / 2;
    }

    return ret;
}

iPoint Map::WorldToMap(int x, int y)
{
    iPoint ret(0, 0);

    if (mapData.type == MapTypes::MAPTYPE_ORTHOGONAL) {
        ret.x = x / mapData.tileWidth;
        ret.y = y / mapData.tileHeight;
    }

    if (mapData.type == MapTypes::MAPTYPE_ISOMETRIC) {
        float half_width = mapData.tileWidth / 2;
        float half_height = mapData.tileHeight / 2;
        ret.x = int((x / half_width + y / half_height) / 2);
        ret.y = int((y / half_height - (x / half_width)) / 2);
    }

    return ret;
}

// Get relative Tile rectangle
SDL_Rect TileSet::GetTileRect(int gid) const
{
    SDL_Rect rect = { 0 };
    int relativeIndex = gid - firstgid;

    rect.w = tileWidth;
    rect.h = tileHeight;
    rect.x = margin + (tileWidth + spacing) * (relativeIndex % columns);
    rect.y = margin + (tileWidth + spacing) * (relativeIndex / columns);

    return rect;
}

TileSet* Map::GetTilesetFromTileId(int gid) const
{
    ListItem<TileSet*>* item = mapData.tilesets.start;
    TileSet* set = NULL;

    while (item)
    {
        set = item->data;
        if (gid < (item->data->firstgid + item->data->tilecount))
        {
            break;
        }
        item = item->next;
    }

    return set;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

	ListItem<TileSet*>* item;
	item = mapData.tilesets.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	mapData.tilesets.Clear();

    // Remove all layers
    ListItem<MapLayer*>* layerItem;
    layerItem = mapData.maplayers.start;

    while (layerItem != NULL)
    {
        RELEASE(layerItem->data);
        layerItem = layerItem->next;
    }

    return true;
}

// Load new map
bool Map::Load(SString mapFileName)
{
    bool ret = true;

    pugi::xml_document mapFileXML;
    pugi::xml_parse_result result = mapFileXML.load_file(mapFileName.GetString());

    if(result == NULL)
    {
        LOG("Could not load map xml file %s. pugi error: %s", mapFileName.GetString(), result.description());
        ret = false;
    }

    if(ret == true)
    {
        ret = LoadMap(mapFileXML);
    }

    if (ret == true)
    {
        ret = LoadTileSet(mapFileXML);
    }

    if (ret == true)
    {
        ret = LoadAllLayers(mapFileXML.child("map"));
    }
    if (ret == true)
    {
        ret = LoadAllObjectGroups(mapFileXML.child("map"));
    }

    if (ret == true)
    {
        ret = LoadCollisionsObject();
    }

    if (ret == true)
    {
        ret = LoadCollisions("Collisions");
    }

    if(ret == true)
    {
        LOG("Successfully parsed map XML file :%s", mapFileName.GetString());
        LOG("width : %d height : %d",mapData.width,mapData.height);
        LOG("tile_width : %d tile_height : %d",mapData.tileWidth, mapData.tileHeight);
        
        LOG("Tilesets----");

        ListItem<TileSet*>* tileset;
        tileset = mapData.tilesets.start;

        while (tileset != NULL) {
            LOG("name : %s firstgid : %d",tileset->data->name.GetString(), tileset->data->firstgid);
            LOG("tile width : %d tile height : %d", tileset->data->tileWidth, tileset->data->tileHeight);
            LOG("spacing : %d margin : %d", tileset->data->spacing, tileset->data->margin);
            tileset = tileset->next;
        }

        LOG("Layers----");

        ListItem<MapLayer*>* mapLayer;
        mapLayer = mapData.maplayers.start;

        while (mapLayer != NULL) {
            LOG("id : %d name : %s", mapLayer->data->id, mapLayer->data->name.GetString());
            LOG("Layer width : %d Layer height : %d", mapLayer->data->width, mapLayer->data->height);
            mapLayer = mapLayer->next;
        }
    }

    // Find the navigation layer
    ListItem<MapLayer*>* mapLayerItem;
    mapLayerItem = mapData.maplayers.start;
    navigationLayer = mapLayerItem->data;

    //Search the layer in the map that contains information for navigation
    while (mapLayerItem != NULL) {
        if (mapLayerItem->data->properties.GetProperty("Navigation") != NULL && mapLayerItem->data->properties.GetProperty("Navigation")->value) {
            navigationLayer = mapLayerItem->data;
            break;
        }
        mapLayerItem = mapLayerItem->next;
    }

    if(mapFileXML) mapFileXML.reset();

    mapLoaded = ret;

    return ret;
}

bool Map::LoadMap(pugi::xml_node mapFile)
{
    bool ret = true;
    pugi::xml_node map = mapFile.child("map");

    if (map == NULL)
    {
        LOG("Error parsing map xml file: Cannot find 'map' tag.");
        ret = false;
    }
    else
    {
        //Load map general properties
        mapData.height = map.attribute("height").as_int();
        mapData.width = map.attribute("width").as_int();
        mapData.tileHeight = map.attribute("tileheight").as_int();
        mapData.tileWidth = map.attribute("tilewidth").as_int();
        mapData.type = MAPTYPE_ORTHOGONAL;
    }

    return ret;
}

bool Map::LoadTileSet(pugi::xml_node mapFile){

    bool ret = true; 

    pugi::xml_node tileset;
    for (tileset = mapFile.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
    {
        TileSet* set = new TileSet();

        set->name = tileset.attribute("name").as_string();
        set->firstgid = tileset.attribute("firstgid").as_int();
        set->margin = tileset.attribute("margin").as_int();
        set->spacing = tileset.attribute("spacing").as_int();
        set->tileWidth = tileset.attribute("tilewidth").as_int();
        set->tileHeight = tileset.attribute("tileheight").as_int();
        set->columns = tileset.attribute("columns").as_int();
        set->tilecount = tileset.attribute("tilecount").as_int();

        SString texPath = path; 
        texPath += tileset.child("image").attribute("source").as_string();
        set->texture = app->tex->Load(texPath.GetString());

        mapData.tilesets.Add(set);
    }

    return ret;
}

bool Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
    bool ret = true;

    //Load the attributes
    layer->id = node.attribute("id").as_int();
    layer->name = node.attribute("name").as_string();
    layer->width = node.attribute("width").as_int();
    layer->height = node.attribute("height").as_int();

    if (node.attribute("parallaxx")) {
        layer->parallaxFactor = node.attribute("parallaxx").as_float();
    }

    else {
        layer->parallaxFactor = 1.0f;
    }
    

    LoadProperties(node, layer->properties);

    //Reserve the memory for the data 
    layer->data = new uint[layer->width * layer->height];
    memset(layer->data, 0, layer->width * layer->height);

    //Iterate over all the tiles and assign the values
    pugi::xml_node tile;
    int i = 0;
    for (tile = node.child("data").child("tile"); tile && ret; tile = tile.next_sibling("tile"))
    {
        layer->data[i] = tile.attribute("gid").as_int();
        i++;
    }

    return ret;
}

bool Map::LoadAllLayers(pugi::xml_node mapNode) {
    bool ret = true;

    for (pugi::xml_node layerNode = mapNode.child("layer"); layerNode && ret; layerNode = layerNode.next_sibling("layer"))
    {
        //Load the layer
        MapLayer* mapLayer = new MapLayer();
        ret = LoadLayer(layerNode, mapLayer);

        //add the layer to the map
        mapData.maplayers.Add(mapLayer);
    }

    return ret;
}

bool Map::LoadObjects(pugi::xml_node& node, MapObjects* objects)
{
    bool ret = true;

    //Load the attributes
    objects->id = node.attribute("id").as_int();
    objects->name = node.attribute("name").as_string();
    objects->width = node.attribute("width").as_int();
    objects->height = node.attribute("height").as_int();

    LoadProperties(node, objects->properties);

    //Reserve the memory for the data 
    //objects->data = new uint[objects->width * objects->height];
    //memset(objects->data, 0, objects->width * objects->height);

    //Iterate over all the tiles and assign the values
    pugi::xml_node object;
    int i = 0;
    for (object = node.child("object"); object && ret; object = object.next_sibling("object"))
    {
        objects->objects.Add(new MapObject{
            object.attribute("id").as_uint(),
            object.attribute("x").as_uint(),
            object.attribute("y").as_uint(),
            object.attribute("width").as_uint(),
            object.attribute("height").as_uint(),

            });
        i++;
    }
    return ret;
}

bool Map::LoadAllObjectGroups(pugi::xml_node mapNode)
{
    bool ret = true;

    for (pugi::xml_node objectNode = mapNode.child("objectgroup"); objectNode && ret; objectNode = objectNode.next_sibling("objectgroup"))
    {
        //Load the layer
        MapObjects* mapObjects = new MapObjects();
        ret = LoadObjects(objectNode, mapObjects);

        //add the layer to the map
        mapData.mapObjects.Add(mapObjects);
    }

    return ret;
}

bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = false;

    for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
    {
        Properties::Property* p = new Properties::Property();
        p->name = propertieNode.attribute("name").as_string();
        p->value = propertieNode.attribute("value").as_bool(); // (!!) I'm assuming that all values are bool !!

        properties.list.Add(p);
    }

    return ret;
}

bool Map::LoadCollisions(std::string collisionLayer)
{
    ListItem<MapLayer*>* mapLayerItem;
    mapLayerItem = mapData.maplayers.start;
    

    while (mapLayerItem != NULL) {

        if (mapLayerItem->data->name.GetString() == collisionLayer) {

            for (int x = 0; x < mapLayerItem->data->width; x++)
            {
                for (int y = 0; y < mapLayerItem->data->height; y++)
                {
                    int gid = mapLayerItem->data->Get(x, y);
                    TileSet* tileset = GetTilesetFromTileId(gid);

                    SDL_Rect r = tileset->GetTileRect(gid);
                    iPoint pos = MapToWorld(x, y);
                    if (gid == tileset->firstgid + 0) {
                        PhysBody* c1 = app->physics->CreateRectangle(pos.x + 12.5, pos.y + 20, 13, 5, STATIC);
                        c1->ctype = ColliderType::SPIKES;
                    }
                   
                }
            }
        }
        mapLayerItem = mapLayerItem->next;

    }

    return true;
  
}

bool Map::LoadCollisionsObject()
{
    ListItem<MapObjects*>* mapObjectsItem;
    mapObjectsItem = mapData.mapObjects.start;
    bool ret = false;

    while (mapObjectsItem != NULL) {
        for (int i = 0; i < mapObjectsItem->data->objects.Count(); i++) {
            MapObject* object = mapObjectsItem->data->objects[i];
            if (mapObjectsItem->data->id == 18) { //floors
                PhysBody* c1 = app->physics->CreateRectangle(object->x + object->height / 2, object->y + object->width / 2, object->height, object->width, STATIC);
                c1->ctype = ColliderType::PLATFORM;
            }
            if (mapObjectsItem->data->id == 19) { //walls
                PhysBody* c1 = app->physics->CreateRectangle(object->x + object->height / 2, object->y + object->width / 2, object->height, object->width, STATIC);
                c1->ctype = ColliderType::WALL;
            }
           
        }
        mapObjectsItem = mapObjectsItem->next;
    }

    return true;
}

void Map::CreateNavigationMap(int& width, int& height, uchar** buffer, MapLayer* navigationLayer) const
{
    bool ret = false;

    //Sets the size of the map. The navigation map is a unidimensional array 
    uchar* navigationMap = new uchar[navigationLayer->width * navigationLayer->height];
    //reserves the memory for the navigation map
    memset(navigationMap, 1, navigationLayer->width * navigationLayer->height);

    for (int x = 0; x < mapData.width; x++)
    {
        for (int y = 0; y < mapData.height; y++)
        {
            //i is the index of x,y coordinate in a unidimensional array that represents the navigation map
            int i = (y * navigationLayer->width) + x;

            //Gets the gid of the map in the navigation layer
            int gid = navigationLayer->Get(x, y);
            TileSet* tileset = GetTilesetFromTileId(gid);
            //If the gid is a blockedGid is an area that I cannot navigate, so is set in the navigation map as 0, all the other areas can be navigated
            //!!!! make sure that you assign blockedGid according to your map

            if (gid == tileset->firstgid + 3)navigationMap[i] = 1;
            else navigationMap[i] = 0;

            /*if (navigationLayer == navigationLayer) {
                if (gid == tileset->firstgid + 1) navigationMap[i] = 1;
                else navigationMap[i] = 0;
            }
            else {
                if (gid == tileset->firstgid) navigationMap[i] = 0;
                else navigationMap[i] = 1;
            }*/


        }
    }

    *buffer = navigationMap;
    width = mapData.width;
    height = mapData.height;
}

bool Map::LoadNavigationLayer()
{
    return false;
}

Properties::Property* Properties::GetProperty(const char* name)
{
    ListItem<Property*>* item = list.start;
    Property* p = NULL;

    while (item)
    {
        if (item->data->name == name) {
            p = item->data;
            break;
        }
        item = item->next;
    }

    return p;
}


