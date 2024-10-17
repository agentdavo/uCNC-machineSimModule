#include "config.h"

int loadConfiguration(const char *filename, ucncAssembly **rootAssembly, ucncLight ***lights, int *lightCount)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open configuration file '%s'.\n", filename);
        return 0;
    }

    // Load the XML tree from the file
    mxml_node_t *tree = mxmlLoadFile(NULL, NULL, file);
    fclose(file);
    if (!tree)
    {
        fprintf(stderr, "Failed to parse XML configuration file.\n");
        return 0;
    }

    // Temporary storage for assemblies and lights
    ucncAssembly **assemblies = NULL;
    int assemblyCount = 0;
    ucncLight **loadedLights = NULL;
    int loadedLightCount = 0;
    char configDir[1024];
    getDirectoryFromPath(filename, configDir);

    // Process the assemblies
    mxml_node_t *assembliesNode = mxmlFindElement(tree, tree, "assemblies", NULL, NULL, MXML_DESCEND_ALL);
    if (assembliesNode)
    {
        for (mxml_node_t *assemblyNode = mxmlFindElement(assembliesNode, assembliesNode, "assembly", NULL, NULL, MXML_DESCEND_ALL);
             assemblyNode;
             assemblyNode = mxmlFindElement(assemblyNode, assembliesNode, "assembly", NULL, NULL, MXML_DESCEND_ALL))
        {
            const char *name = mxmlElementGetAttr(assemblyNode, "name");
            const char *parentName = mxmlElementGetAttr(assemblyNode, "parent");

            // Origin
            float originX = 0.0f, originY = 0.0f, originZ = 0.0f;
            mxml_node_t *originNode = mxmlFindElement(assemblyNode, assemblyNode, "origin", NULL, NULL, MXML_DESCEND_ALL);
            if (originNode)
            {
                originX = atof(mxmlElementGetAttr(originNode, "x"));
                originY = atof(mxmlElementGetAttr(originNode, "y"));
                originZ = atof(mxmlElementGetAttr(originNode, "z"));
            }

            // Position
            float positionX = 0.0f, positionY = 0.0f, positionZ = 0.0f;
            mxml_node_t *positionNode = mxmlFindElement(assemblyNode, assemblyNode, "position", NULL, NULL, MXML_DESCEND_ALL);
            if (positionNode)
            {
                positionX = atof(mxmlElementGetAttr(positionNode, "x"));
                positionY = atof(mxmlElementGetAttr(positionNode, "y"));
                positionZ = atof(mxmlElementGetAttr(positionNode, "z"));
            }

            // Rotation
            float rotationX = 0.0f, rotationY = 0.0f, rotationZ = 0.0f;
            mxml_node_t *rotationNode = mxmlFindElement(assemblyNode, assemblyNode, "rotation", NULL, NULL, MXML_DESCEND_ALL);
            if (rotationNode)
            {
                rotationX = atof(mxmlElementGetAttr(rotationNode, "x"));
                rotationY = atof(mxmlElementGetAttr(rotationNode, "y"));
                rotationZ = atof(mxmlElementGetAttr(rotationNode, "z"));
            }

            // Color
            float colorR = 1.0f, colorG = 1.0f, colorB = 1.0f;
            mxml_node_t *colorNode = mxmlFindElement(assemblyNode, assemblyNode, "color", NULL, NULL, MXML_DESCEND_ALL);
            if (colorNode)
            {
                colorR = atof(mxmlElementGetAttr(colorNode, "r"));
                colorG = atof(mxmlElementGetAttr(colorNode, "g"));
                colorB = atof(mxmlElementGetAttr(colorNode, "b"));
            }

            // Motion Type, Axis, and Invert Motion
            const char *motionType = MOTION_TYPE_NONE; // Default to none
            char motionAxis = ' ';
            int invertMotion = 0;
            mxml_node_t *motionNode = mxmlFindElement(assemblyNode, assemblyNode, "motion", NULL, NULL, MXML_DESCEND_ALL);
            if (motionNode)
            {
                motionType = mxmlElementGetAttr(motionNode, "type");
                const char *axisStr = mxmlElementGetAttr(motionNode, "axis");
                if (axisStr)
                    motionAxis = axisStr[0];
                const char *invertStr = mxmlElementGetAttr(motionNode, "invert");
                if (invertStr && strcmp(invertStr, "yes") == 0)
                {
                    invertMotion = 1;
                }
            }

            // Home position and rotation
            float homePositionX = 0.0f, homePositionY = 0.0f, homePositionZ = 0.0f;
            float homeRotationX = 0.0f, homeRotationY = 0.0f, homeRotationZ = 0.0f;
            mxml_node_t *homeNode = mxmlFindElement(assemblyNode, assemblyNode, "home", NULL, NULL, MXML_DESCEND_ALL);
            if (homeNode)
            {
                mxml_node_t *homePositionNode = mxmlFindElement(homeNode, homeNode, "position", NULL, NULL, MXML_DESCEND_ALL);
                if (homePositionNode)
                {
                    homePositionX = atof(mxmlElementGetAttr(homePositionNode, "x"));
                    homePositionY = atof(mxmlElementGetAttr(homePositionNode, "y"));
                    homePositionZ = atof(mxmlElementGetAttr(homePositionNode, "z"));
                }
                mxml_node_t *homeRotationNode = mxmlFindElement(homeNode, homeNode, "rotation", NULL, NULL, MXML_DESCEND_ALL);
                if (homeRotationNode)
                {
                    homeRotationX = atof(mxmlElementGetAttr(homeRotationNode, "x"));
                    homeRotationY = atof(mxmlElementGetAttr(homeRotationNode, "y"));
                    homeRotationZ = atof(mxmlElementGetAttr(homeRotationNode, "z"));
                }
            }

            // Create the assembly object
            ucncAssembly *assembly = ucncAssemblyNew(
                name, parentName,
                originX, originY, originZ, positionX, positionY, positionZ,
                rotationX, rotationY, rotationZ,
                homePositionX, homePositionY, homePositionZ,
                homeRotationX, homeRotationY, homeRotationZ,
                colorR, colorG, colorB,
                motionType, motionAxis, invertMotion);

            if (assembly)
            {
                // Reallocation with error checking
                ucncAssembly **temp = realloc(assemblies, (assemblyCount + 1) * sizeof(ucncAssembly *));
                if (!temp)
                {
                    fprintf(stderr, "Reallocation failed for assemblies.\n");
                    ucncAssemblyFree(assembly);
                    for (int i = 0; i < assemblyCount; i++)
                    {
                        ucncAssemblyFree(assemblies[i]);
                    }
                    free(assemblies);
                    mxmlDelete(tree);
                    return 0;
                }
                assemblies = temp;
                assemblies[assemblyCount++] = assembly;
                printf("Loaded Assembly: %s\n", name);
            }
        }
    }

    // Process actors
    mxml_node_t *actorsNode = mxmlFindElement(tree, tree, "actors", NULL, NULL, MXML_DESCEND_ALL);
    if (actorsNode)
    {
        for (mxml_node_t *actorNode = mxmlFindElement(actorsNode, actorsNode, "actor", NULL, NULL, MXML_DESCEND_ALL);
             actorNode;
             actorNode = mxmlFindElement(actorNode, actorsNode, "actor", NULL, NULL, MXML_DESCEND_ALL))
        {
            // Get the name, assembly, and STL file for the actor
            const char *name = mxmlElementGetAttr(actorNode, "name");
            const char *assemblyName = mxmlElementGetAttr(actorNode, "assembly");
            const char *stlFile = mxmlElementGetAttr(actorNode, "stlFile");

            if (!name || !assemblyName || !stlFile)
            {
                fprintf(stderr, "Missing attribute (name, assembly, or stlFile) in actor.\n");
                continue;
            }

            // Process color
            float colorR = 1.0f, colorG = 1.0f, colorB = 1.0f;
            mxml_node_t *colorNode = mxmlFindElement(actorNode, actorNode, "color", NULL, NULL, MXML_DESCEND_ALL);
            if (colorNode)
            {
                colorR = atof(mxmlElementGetAttr(colorNode, "r"));
                colorG = atof(mxmlElementGetAttr(colorNode, "g"));
                colorB = atof(mxmlElementGetAttr(colorNode, "b"));
            }

            // Create a new actor object
            ucncActor *actor = ucncActorNew(name, stlFile, colorR, colorG, colorB, configDir);
            if (!actor)
            {
                fprintf(stderr, "Failed to create actor '%s'.\n", name);
                continue;
            }

            // Find the parent assembly by name
            ucncAssembly *parentAssembly = NULL;
            for (int i = 0; i < assemblyCount; i++)
            {
                if (strcmp(assemblies[i]->name, assemblyName) == 0)
                {
                    parentAssembly = assemblies[i];
                    break;
                }
            }

            if (!parentAssembly)
            {
                fprintf(stderr, "Parent assembly '%s' not found for actor '%s'.\n", assemblyName, name);
                ucncActorFree(actor);
                continue;
            }

            // Add actor to the parent assembly
            if (!ucncAssemblyAddActor(parentAssembly, actor))
            {
                fprintf(stderr, "Failed to add actor '%s' to assembly '%s'.\n", name, assemblyName);
                ucncActorFree(actor);
                continue;
            }
        }
    }

    // Process lights
    mxml_node_t *lightsNode = mxmlFindElement(tree, tree, "lights", NULL, NULL, MXML_DESCEND_ALL);
    if (lightsNode)
    {
        for (mxml_node_t *lightNode = mxmlFindElement(lightsNode, lightsNode, "light", NULL, NULL, MXML_DESCEND_ALL);
             lightNode;
             lightNode = mxmlFindElement(lightNode, lightsNode, "light", NULL, NULL, MXML_DESCEND_ALL))
        {
            const char *lightID_str = mxmlElementGetAttr(lightNode, "id");
            float posX = atof(mxmlElementGetAttr(lightNode, "x"));
            float posY = atof(mxmlElementGetAttr(lightNode, "y"));
            float posZ = atof(mxmlElementGetAttr(lightNode, "z"));

            // Ambient
            mxml_node_t *ambientNode = mxmlFindElement(lightNode, lightNode, "ambient", NULL, NULL, MXML_DESCEND_ALL);
            float ambientR = atof(mxmlElementGetAttr(ambientNode, "r"));
            float ambientG = atof(mxmlElementGetAttr(ambientNode, "g"));
            float ambientB = atof(mxmlElementGetAttr(ambientNode, "b"));

            // Diffuse
            mxml_node_t *diffuseNode = mxmlFindElement(lightNode, lightNode, "diffuse", NULL, NULL, MXML_DESCEND_ALL);
            float diffuseR = atof(mxmlElementGetAttr(diffuseNode, "r"));
            float diffuseG = atof(mxmlElementGetAttr(diffuseNode, "g"));
            float diffuseB = atof(mxmlElementGetAttr(diffuseNode, "b"));

            // Specular
            mxml_node_t *specularNode = mxmlFindElement(lightNode, lightNode, "specular", NULL, NULL, MXML_DESCEND_ALL);
            float specularR = atof(mxmlElementGetAttr(specularNode, "r"));
            float specularG = atof(mxmlElementGetAttr(specularNode, "g"));
            float specularB = atof(mxmlElementGetAttr(specularNode, "b"));

            GLenum lightID;
            if (strcmp(lightID_str, "GL_LIGHT0") == 0)
                lightID = GL_LIGHT0;
            else if (strcmp(lightID_str, "GL_LIGHT1") == 0)
                lightID = GL_LIGHT1;
            else
            {
                fprintf(stderr, "Unknown light ID '%s'.\n", lightID_str);
                continue;
            }

            // Create light object
            ucncLight *light = ucncLightNew(lightID, posX, posY, posZ, ambientR, ambientG, ambientB, diffuseR, diffuseG, diffuseB, specularR, specularG, specularB);
            if (light)
            {
                // Reallocation for lights
                ucncLight **temp = realloc(loadedLights, (loadedLightCount + 1) * sizeof(ucncLight *));
                if (!temp)
                {
                    fprintf(stderr, "Reallocation failed for lights.\n");
                    ucncLightFree(light);
                    freeAllLights(loadedLights, loadedLightCount);
                    mxmlDelete(tree);
                    return 0;
                }
                loadedLights = temp;
                loadedLights[loadedLightCount++] = light;
            }
        }
    }

    mxmlDelete(tree); // Free the XML tree

    *lights = loadedLights;
    *lightCount = loadedLightCount;

    // Assign root assembly and link child-parent relationships
    if (*rootAssembly == NULL)
    {
        // Create the default root assembly if no root is found
        *rootAssembly = ucncAssemblyNew(
            "root", NULL,
            0.0f, 0.0f, 0.0f, // Default origin
            0.0f, 0.0f, 0.0f, // Default position
            0.0f, 0.0f, 0.0f, // Default rotation
            0.0f, 0.0f, 0.0f, // Home position
            0.0f, 0.0f, 0.0f, // Home rotation
            1.0f, 1.0f, 1.0f, // Default white color
            "none",           // No motion
            ' ',              // No motion axis
            0                 // No inverted motion
        );
    }

    for (int i = 0; i < assemblyCount; i++)
    {
        ucncAssembly *current = assemblies[i];
        if (strcmp(current->parentName, "NULL") == 0)
        {
            *rootAssembly = current;
        }
        else
        {
            for (int j = 0; j < assemblyCount; j++)
            {
                if (strcmp(assemblies[j]->name, current->parentName) == 0)
                {
                    ucncAssemblyAddAssembly(assemblies[j], current);
                    break;
                }
            }
        }
    }

    if (*rootAssembly == NULL)
    {
        fprintf(stderr, "No root assembly found or created.\n");
        freeAllLights(loadedLights, loadedLightCount);
        for (int i = 0; i < assemblyCount; i++)
        {
            ucncAssemblyFree(assemblies[i]);
        }
        free(assemblies);
        return 0;
    }

    free(assemblies); // Free the temporary assemblies array after setting the hierarchy
    return 1;         // Success
}
