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
    // Process <lights> node
    mxml_node_t *lightsNode = mxmlFindElement(tree, tree, "lights", NULL, NULL, MXML_DESCEND_ALL);
    if (lightsNode) {
        // Iterate over each <light> node
        for (mxml_node_t *lightNode = mxmlFindElement(lightsNode, lightsNode, "light", NULL, NULL, MXML_DESCEND_ALL);
             lightNode;
             lightNode = mxmlFindElement(lightNode, lightsNode, "light", NULL, NULL, MXML_DESCEND_ALL)) {

            // Extract light ID
            const char *lightID_str = mxmlElementGetAttr(lightNode, "id");
            if (!lightID_str) {
                fprintf(stderr, "Light without 'id' attribute found. Skipping.\n");
                continue;
            }

            GLenum lightID;
            if (strcmp(lightID_str, "GL_LIGHT0") == 0)
                lightID = GL_LIGHT0;
            else if (strcmp(lightID_str, "GL_LIGHT1") == 0)
                lightID = GL_LIGHT1;
            else if (strcmp(lightID_str, "GL_LIGHT2") == 0)
                lightID = GL_LIGHT2;
            else if (strcmp(lightID_str, "GL_LIGHT3") == 0)
                lightID = GL_LIGHT3;
            else if (strcmp(lightID_str, "GL_LIGHT4") == 0)
                lightID = GL_LIGHT4;
            else if (strcmp(lightID_str, "GL_LIGHT5") == 0)
                lightID = GL_LIGHT5;
            else if (strcmp(lightID_str, "GL_LIGHT6") == 0)
                lightID = GL_LIGHT6;
            else if (strcmp(lightID_str, "GL_LIGHT7") == 0)
                lightID = GL_LIGHT7;
            else {
                fprintf(stderr, "Unknown light ID '%s'. Skipping.\n", lightID_str);
                continue;
            }

            // Extract position
            mxml_node_t *positionNode = mxmlFindElement(lightNode, lightNode, "position", NULL, NULL, MXML_DESCEND_ALL);
            if (!positionNode) {
                fprintf(stderr, "Light '%s' missing <position> tag. Skipping.\n", lightID_str);
                continue;
            }
            const char *posX_str = mxmlElementGetAttr(positionNode, "x");
            const char *posY_str = mxmlElementGetAttr(positionNode, "y");
            const char *posZ_str = mxmlElementGetAttr(positionNode, "z");
            const char *posW_str = mxmlElementGetAttr(positionNode, "w");

            if (!posX_str || !posY_str || !posZ_str || !posW_str) {
                fprintf(stderr, "Light '%s' has incomplete <position> attributes. Skipping.\n", lightID_str);
                continue;
            }

            float posX = atof(posX_str);
            float posY = atof(posY_str);
            float posZ = atof(posZ_str);
            float posW = atof(posW_str);

            // Extract ambient color
            mxml_node_t *ambientNode = mxmlFindElement(lightNode, lightNode, "ambient", NULL, NULL, MXML_DESCEND_ALL);
            if (!ambientNode) {
                fprintf(stderr, "Light '%s' missing <ambient> tag. Using default ambient (0,0,0).\n", lightID_str);
            }
            float ambientR = ambientNode ? atof(mxmlElementGetAttr(ambientNode, "r")) : 0.0f;
            float ambientG = ambientNode ? atof(mxmlElementGetAttr(ambientNode, "g")) : 0.0f;
            float ambientB = ambientNode ? atof(mxmlElementGetAttr(ambientNode, "b")) : 0.0f;

            // Extract diffuse color
            mxml_node_t *diffuseNode = mxmlFindElement(lightNode, lightNode, "diffuse", NULL, NULL, MXML_DESCEND_ALL);
            if (!diffuseNode) {
                fprintf(stderr, "Light '%s' missing <diffuse> tag. Using default diffuse (1,1,1).\n", lightID_str);
            }
            float diffuseR = diffuseNode ? atof(mxmlElementGetAttr(diffuseNode, "r")) : 1.0f;
            float diffuseG = diffuseNode ? atof(mxmlElementGetAttr(diffuseNode, "g")) : 1.0f;
            float diffuseB = diffuseNode ? atof(mxmlElementGetAttr(diffuseNode, "b")) : 1.0f;

            // Extract specular color
            mxml_node_t *specularNode = mxmlFindElement(lightNode, lightNode, "specular", NULL, NULL, MXML_DESCEND_ALL);
            if (!specularNode) {
                fprintf(stderr, "Light '%s' missing <specular> tag. Using default specular (1,1,1).\n", lightID_str);
            }
            float specularR = specularNode ? atof(mxmlElementGetAttr(specularNode, "r")) : 1.0f;
            float specularG = specularNode ? atof(mxmlElementGetAttr(specularNode, "g")) : 1.0f;
            float specularB = specularNode ? atof(mxmlElementGetAttr(specularNode, "b")) : 1.0f;

            // Create light object
            ucncLight *light = ucncLightNew(lightID, posX, posY, posZ,
                                           ambientR, ambientG, ambientB,
                                           diffuseR, diffuseG, diffuseB,
                                           specularR, specularG, specularB);
            if (!light) {
                fprintf(stderr, "Failed to create light '%s'. Skipping.\n", lightID_str);
                continue;
            }

            // Determine if the light is a spotlight
            mxml_node_t *spotNode = mxmlFindElement(lightNode, lightNode, "spot", NULL, NULL, MXML_DESCEND_ALL);
            if (spotNode) {
                mxml_node_t *directionNode = mxmlFindElement(spotNode, spotNode, "direction", NULL, NULL, MXML_DESCEND_ALL);
                if (!directionNode) {
                    fprintf(stderr, "Light '%s' has <spot> without <direction>. Ignoring spotlight properties.\n", lightID_str);
                } else {
                    const char *dirX_str = mxmlElementGetAttr(directionNode, "x");
                    const char *dirY_str = mxmlElementGetAttr(directionNode, "y");
                    const char *dirZ_str = mxmlElementGetAttr(directionNode, "z");

                    if (!dirX_str || !dirY_str || !dirZ_str) {
                        fprintf(stderr, "Light '%s' has incomplete <direction> attributes. Ignoring spotlight properties.\n", lightID_str);
                    } else {
                        light->spot_direction[0] = atof(dirX_str);
                        light->spot_direction[1] = atof(dirY_str);
                        light->spot_direction[2] = atof(dirZ_str);
                        light->is_spotlight = 1;
                    }
                }

                // Extract spotlight cutoff
                const char *cutoff_str = mxmlElementGetAttr(spotNode, "cutoff");
                if (cutoff_str) {
                    light->spot_cutoff = atof(cutoff_str);
                } else {
                    light->spot_cutoff = 180.0f; // Default: no spotlight effect
                }

                // Extract spotlight exponent
                const char *exponent_str = mxmlElementGetAttr(spotNode, "exponent");
                if (exponent_str) {
                    light->spot_exponent = atof(exponent_str);
                } else {
                    light->spot_exponent = 0.0f; // Default
                }
            }

            // Extract attenuation properties
            mxml_node_t *attenuationNode = mxmlFindElement(lightNode, lightNode, "attenuation", NULL, NULL, MXML_DESCEND_ALL);
            if (attenuationNode) {
                mxml_node_t *constantNode = mxmlFindElement(attenuationNode, attenuationNode, "constant", NULL, NULL, MXML_DESCEND_ALL);
                if (constantNode) {
                    const char *const_val_str = mxmlElementGetAttr(constantNode, "value");
                    if (const_val_str)
                        light->constant_attenuation = atof(const_val_str);
                }

                mxml_node_t *linearNode = mxmlFindElement(attenuationNode, attenuationNode, "linear", NULL, NULL, MXML_DESCEND_ALL);
                if (linearNode) {
                    const char *linear_val_str = mxmlElementGetAttr(linearNode, "value");
                    if (linear_val_str)
                        light->linear_attenuation = atof(linear_val_str);
                }

                mxml_node_t *quadraticNode = mxmlFindElement(attenuationNode, attenuationNode, "quadratic", NULL, NULL, MXML_DESCEND_ALL);
                if (quadraticNode) {
                    const char *quadratic_val_str = mxmlElementGetAttr(quadraticNode, "value");
                    if (quadratic_val_str)
                        light->quadratic_attenuation = atof(quadratic_val_str);
                }
            }

            // Add the light to the loadedLights array
            ucncLight **temp = realloc(loadedLights, (loadedLightCount + 1) * sizeof(ucncLight *));
            if (!temp) {
                fprintf(stderr, "Reallocation failed while loading lights. Freeing allocated lights.\n");
                ucncLightFree(light);
                freeAllLights(&loadedLights, loadedLightCount);
                mxmlDelete(tree);
                return -1;
            }
            loadedLights = temp;
            loadedLights[loadedLightCount++] = light;
        }
    }

    // Clean up
    mxmlDelete(tree);

    // Assign outputs
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
        freeAllLights(&loadedLights, loadedLightCount);
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
