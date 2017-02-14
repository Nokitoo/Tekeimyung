/**
* @Author   Guillaume Labey
*/

#include <Engine/Utils/Logger.hpp>
#include <dirent.h>
#include <fstream>
#include <algorithm>
#include <vector>

#include <Engine/Utils/Exception.hpp>
#include <Engine/Utils/Helper.hpp>
#include <Engine/Sound/SoundManager.hpp>

#include <Engine/Utils/RessourceManager.hpp>

RessourceManager*   RessourceManager::_ressourceManager = nullptr;

RessourceManager::RessourceManager() {}

RessourceManager::~RessourceManager() {}

void    RessourceManager::loadResources(const std::string& directory)
{
    DIR* dir;
    struct dirent* ent;
    RessourceManager* ressourceManager = RessourceManager::getInstance();
    std::vector<std::string> texturesExtensions = { TEXTURES_EXT };
    std::vector<std::string> modelsExtensions = { MODELS_EXT };
    std::vector<std::string> soundsExtensions = { SOUNDS_EXT };

    dir = opendir(directory.c_str());
    if (!dir)
        EXCEPT(FileNotFoundException, "Cannot open resource directory \"%s\"", directory.c_str());

    while ((ent = readdir(dir)) != NULL)
    {
        // No file extension, is directory
        if (std::string(ent->d_name).find(".") == std::string::npos)
        {
            // Load directory
            std::string directoryPath = std::string(directory).append("/").append(ent->d_name);
            loadResources(directoryPath);
        }
        else
        {
            std::string file = std::string(directory).append("/").append(ent->d_name);
            std::string extension = RessourceManager::getFileExtension(ent->d_name);
            std::string basename = getBasename(file);

            extension = Helper::lowerCaseString(extension);
            // Texture resource
            if (std::find(texturesExtensions.cbegin(), texturesExtensions.cend(), extension) != texturesExtensions.cend())
            {
                loadResource<Texture>(basename, file);
            }
            // Model resource
            else if (std::find(modelsExtensions.cbegin(), modelsExtensions.cend(), extension) != modelsExtensions.cend())
            {
                loadResource<Model>(basename, file);
            }
            else if (std::find(soundsExtensions.cbegin(), soundsExtensions.cend(), extension) != soundsExtensions.cend())
            {
                loadSound(basename, file);
            }
        }
    }

    closedir(dir);
}

RessourceManager*   RessourceManager::getInstance()
{
    if (!_ressourceManager)
    {
        _ressourceManager = new RessourceManager();
    }

    return (_ressourceManager);
}

std::string  RessourceManager::getFileExtension(const std::string& fileName)
{
    std::size_t size = fileName.rfind('.', fileName.length());
    if (size != std::string::npos)
        return (fileName.substr(size + 1, fileName.length() - size));

    return ("");
}

std::string RessourceManager::getBasename(const std::string& fileName)
{
    size_t basenameOcur;
    std::string basename = fileName;

    basenameOcur = fileName.find_last_of('/');
    if (basenameOcur != std::string::npos)
    {
        basename = fileName.substr(basenameOcur + 1);
    }

    return (basename);
}

/*
** File
*/

void    RessourceManager::createFile(const std::string& fileName, const std::string& fileContent)
{
    std::ofstream file;
    RessourceManager::sFile fileInfos;
    std::string basename = getBasename(fileName);

    // Check file does not exists
    file.open(fileName.c_str());
    if (!file.good())
        EXCEPT(InternalErrorException, "Attempt to create the file %s which already exists", fileName.c_str());

    // Open file to write
    file = std::ofstream(fileName.c_str(), std::ios::out);
    if (!file.good())
        EXCEPT(InternalErrorException, "Failed to create the file %s", fileName.c_str());

    // Save content
    file << fileContent;
    file.close();

    // Save file in RessourceManager
    fileInfos.path = fileName;
    fileInfos.content = std::string(fileContent.begin(), fileContent.end());
    fileInfos.basename = basename;
    _files[basename] = fileInfos;
}

std::string RessourceManager::getFile(const std::string& fileName)
{
    // Textures are stored with their basename
    std::string basename = getBasename(fileName);

    // The file is not loaded
    if (_files.find(basename) == _files.end())
    {
        return (loadFile(basename, fileName));
    }

    auto &&file = _files.at(basename);
    return (file.content);
}

void        RessourceManager::saveFile(const std::string& fileName, const std::string fileContent)
{
    std::ofstream file;
    RessourceManager::sFile fileInfos;
    std::string basename = getBasename(fileName);

    file.open(fileName.c_str(), std::ios::trunc);
    if (!file.good())
        EXCEPT(FileNotFoundException, "Failed to open file \"%s\"", fileName.c_str());

    file << fileContent;
    file.close();

    // The file does not exist is RessourceManager
    if (_files.find(basename) != _files.end())
    {
        // Init file informations
        fileInfos.content = fileContent;
        fileInfos.path = fileName;
        fileInfos.basename = basename;
    }
    else
    {
        // Update file informations
        fileInfos.content = fileContent;
    }
    _files[basename] = fileInfos;
}

std::string RessourceManager::loadFile(const std::string basename, const std::string& fileName)
{
    std::ifstream               file;
    int                         fileSize;
    std::vector<char>           fileContent;
    RessourceManager::sFile     fileInfos;

    file.open(fileName.c_str());
    if (!file.good())
        EXCEPT(FileNotFoundException, "Failed to open file \"%s\"", fileName.c_str());

    file.seekg(0, file.end);
    fileSize = static_cast<int>(file.tellg());
    file.seekg(0, file.beg);

    fileContent.resize(fileSize);
    file.read(fileContent.data(), fileSize);
    file.close();

    fileInfos.path = fileName;
    fileInfos.content = std::string(fileContent.begin(), fileContent.end());
    fileInfos.basename = basename;
    _files[basename] = fileInfos;
    return (_files[basename].content);
}

const std::vector<RessourceManager::sSoundStrings>&  RessourceManager::getSoundsStrings() const
{
    return (_soundsStrings);
}

void    RessourceManager::loadSound(const std::string basename, const std::string& fileName)
{
    // Add sound string to Ressource Manager
    sSoundStrings soundString;
    soundString.path = fileName;
    soundString.name = basename;
    _soundsStrings.push_back(soundString);

    // Load sound in Sound Manager
    SoundManager::getInstance()->registerSound(fileName);
}

template<typename T>
T*  RessourceManager::getResource(const std::string& fileName)
{
    // Resources are stored with their basename
    std::string basename = getBasename(fileName);

    // The resource is not loaded
    auto& resourceMap = _resources[T::getResourceType()];
    if (resourceMap.find(basename) == resourceMap.end())
    {
        return loadResource<T>(basename, fileName);
    }

    auto& resource = resourceMap.at(basename);
    return (static_cast<T*>(resource.get()));
}

template<typename T>
T*  RessourceManager::loadResource(const std::string& basename, const std::string& fileName)
{
    std::unique_ptr<T> resource = std::make_unique<T>();

    if (!resource->loadFromFile(fileName))
    {
        return (nullptr);
    }

    auto& resourceMap = _resources[T::getResourceType()];

    resource->setId(basename);
    resource->setPath(fileName);
    _resourcesNames[T::getResourceType()].push_back(resource->getId().c_str());
    resourceMap[basename] = std::move(resource);
    return (static_cast<T*>(resourceMap[basename].get()));
}

template<typename T>
T*  RessourceManager::registerResource(const std::string& basename, std::unique_ptr<T> resource)
{
    auto& resourceMap = _resources[T::getResourceType()];

    resource->setId(basename);
    resourceMap[basename] = std::move(resource);

    return (static_cast<T*>(resourceMap[basename].get()));
}

template<typename T>
const std::vector<const char*>& RessourceManager::getResourcesNames()
{
    return _resourcesNames[T::getResourceType()];
}

template Model*  RessourceManager::getResource<Model>(const std::string& fileName);
template Texture*  RessourceManager::getResource<Texture>(const std::string& fileName);
template Geometry*  RessourceManager::getResource<Geometry>(const std::string& fileName);

template Model*  RessourceManager::registerResource<Model>(const std::string& basename, std::unique_ptr<Model> resource);
template Texture*  RessourceManager::registerResource<Texture>(const std::string& basename, std::unique_ptr<Texture> resource);
template Geometry*  RessourceManager::registerResource<Geometry>(const std::string& basename, std::unique_ptr<Geometry> resource);

template const std::vector<const char*>&  RessourceManager::getResourcesNames<Model>();
template const std::vector<const char*>&  RessourceManager::getResourcesNames<Texture>();
