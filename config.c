#include "config.h"
#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void freeLights(ucncLight **lights, int lightCount) {
    if (!lights) return;
    for (int i = 0; i < lightCount; i++) {
        ucncLightFree(lights[i]);
    }
    free(lights);
}

int loadConfiguration(const char *filename, ucncAssembly **rootAssembly, ucncLight ***lights, int *lightCount) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open configuration file '%s'.\n", filename);
        return 0;
    }

    mxml_node_t *tree = mxmlLoadFile(NULL, file, MXML_OPAQUE_CALLBACK);
    fclose(file);
    if (!tree) {
        fprintf(stderr, "Failed to parse XML configuration file.\n");
        return 0;
    }

    // Temporary storage for assemblies and lights
    ucncAssembly **assemblies = NULL;
    int assemblyCount = 0;
    ucncLight **loadedLights = NULL;
    int loadedLightCount = 0;

    // Process the assemblies
    mxml_node_t *assembliesNode = mxmlFindElement(tree, tree, "assemblies", NULL, NULL, MXML_DESCEND);
    if (assembliesNode) {
        for (mxml_node_t *assemblyNode = mxmlFindElement(assembliesNode, assembliesNode, "assembly", NULL, NULL, MXML_DESCEND);
             assemblyNode;
             assemblyNode = mxmlFindElement(assemblyNode, assembliesNode, "assembly", NULL, NULL, MXML_NO_DESCEND)) {

            const char *name = mxmlElementGetAttr(assemblyNode, "name");
            const char *parentName = mxmlElementGetAttr(assemblyNode, "parent");

            // Origin
            mxml_node_t *originNode = mxmlFindElement(assemblyNode, assemblyNode, "origin", NULL, NULL, MXML_DESCEND);
            float originX = 0.0f, originY = 0.0f, originZ = 0.0f;
            if (originNode) {
                originX = atof(mxmlElementGetAttr(originNode, "x"));
                originY = atof(mxmlElementGetAttr(originNode, "y"));
                originZ = atof(mxmlElementGetAttr(originNode, "z"));
            } else {
                fprintf(stderr, "Missing origin for assembly '%s'. Using default values.\n", name);
            }

            // Position
            mxml_node_t *positionNode = mxmlFindElement(assemblyNode, assemblyNode, "position", NULL, NULL, MXML_DESCEND);
            float positionX = 0.0f, positionY = 0.0f, positionZ = 0.0f;
            if (positionNode) {
                positionX = atof(mxmlElementGetAttr(positionNode, "x"));
                positionY = atof(mxmlElementGetAttr(positionNode, "y"));
                positionZ = atof(mxmlElementGetAttr(positionNode, "z"));
            } else {
                fprintf(stderr, "Missing position for assembly '%s'. Using default values.\n", name);
            }

            // Rotation
            mxml_node_t *rotationNode = mxmlFindElement(assemblyNode, assemblyNode, "rotation", NULL, NULL, MXML_DESCEND);
            float rotationX = 0.0f, rotationY = 0.0f, rotationZ = 0.0f;
            if (rotationNode) {
                rotationX = atof(mxmlElementGetAttr(rotationNode, "x"));
                rotationY = atof(mxmlElementGetAttr(rotationNode, "y"));
                rotationZ = atof(mxmlElementGetAttr(rotationNode, "z"));
            } else {
                fprintf(stderr, "Missing rotation for assembly '%s'. Using default values.\n", name);
            }

            // Color
            mxml_node_t *colorNode = mxmlFindElement(assemblyNode, assemblyNode, "color", NULL, NULL, MXML_DESCEND);
            float colorR = 1.0f, colorG = 1.0f, colorB = 1.0f;
            if (colorNode) {
                colorR = atof(mxmlElementGetAttr(colorNode, "r"));
                colorG = atof(mxmlElementGetAttr(colorNode, "g"));
                colorB = atof(mxmlElementGetAttr(colorNode, "b"));
            } else {
                fprintf(stderr, "Missing color for assembly '%s'. Using default values.\n", name);
            }

            ucncAssembly *assembly = ucncAssemblyNew(name, strcmp(parentName, "NULL") == 0 ? NULL : parentName,
                                                     originX, originY, originZ, positionX, positionY, positionZ,
                                                     rotationX, rotationY, rotationZ, colorR, colorG, colorB);

            if (assembly) {
                ucncAssembly **temp = realloc(assemblies, (assemblyCount + 1) * sizeof(ucncAssembly*));
                if (!temp) {
                    fprintf(stderr, "Reallocation failed for assemblies.\n");
                    ucncAssemblyFree(assembly);
                    for (int i = 0; i < assemblyCount; i++) {
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

    // Process the actors
    mxml_node_t *actorsNode = mxmlFindElement(tree, tree, "actors", NULL, NULL, MXML_DESCEND);
    if (actorsNode) {
        for (mxml_node_t *actorNode = mxmlFindElement(actorsNode, actorsNode, "actor", NULL, NULL, MXML_DESCEND);
             actorNode;
             actorNode = mxmlFindElement(actorNode, actorsNode, "actor", NULL, NULL, MXML_NO_DESCEND)) {

            const char *name = mxmlElementGetAttr(actorNode, "name");
            const char *assemblyName = mxmlElementGetAttr(actorNode, "assembly");
            const char *stlFile = mxmlElementGetAttr(actorNode, "stlFile");

            // Color
            mxml_node_t *colorNode = mxmlFindElement(actorNode, actorNode, "color", NULL, NULL, MXML_DESCEND);
            float colorR = 1.0f, colorG = 1.0f, colorB = 1.0f;
            if (colorNode) {
                colorR = atof(mxmlElementGetAttr(colorNode, "r"));
                colorG = atof(mxmlElementGetAttr(colorNode, "g"));
                colorB = atof(mxmlElementGetAttr(colorNode, "b"));
            }

            ucncActor *actor = ucncActorNew(name, stlFile, colorR, colorG, colorB);
            if (!actor) {
                fprintf(stderr, "Failed to create actor '%s'.\n", name);
                continue;
            }

            // Find the parent assembly
            ucncAssembly *parentAssembly = NULL;
            for (int i = 0; i < assemblyCount; i++) {
                if (strcmp(assemblies[i]->name, assemblyName) == 0) {
                    parentAssembly = assemblies[i];
                    break;
                }
            }

            if (!parentAssembly) {
                fprintf(stderr, "Parent assembly '%s' not found for actor '%s'.\n", assemblyName, name);
                ucncActorFree(actor);
                continue;
            }

            if (!ucncAssemblyAddActor(parentAssembly, actor)) {
                fprintf(stderr, "Failed to add actor '%s' to assembly '%s'.\n", name, assemblyName);
                ucncActorFree(actor);
                continue;
            }
        }
    }

    // Process the lights
    mxml_node_t *lightsNode = mxmlFindElement(tree, tree, "lights", NULL, NULL, MXML_DESCEND);
    if (lightsNode) {
        for (mxml_node_t *lightNode = mxmlFindElement(lightsNode, lightsNode, "light", NULL, NULL, MXML_DESCEND);
             lightNode;
             lightNode = mxmlFindElement(lightNode, lightsNode, "light", NULL, NULL, MXML_NO_DESCEND)) {

            const char *lightID_str = mxmlElementGetAttr(lightNode, "id");
            float posX = atof(mxmlElementGetAttr(lightNode, "x"));
            float posY = atof(mxmlElementGetAttr(lightNode, "y"));
            float posZ = atof(mxmlElementGetAttr(lightNode, "z"));

            // Ambient
            mxml_node_t *ambientNode = mxmlFindElement(lightNode, lightNode, "ambient", NULL, NULL, MXML_DESCEND);
            float ambientR = atof(mxmlElementGetAttr(ambientNode, "r"));
            float ambientG = atof(mxmlElementGetAttr(ambientNode, "g"));
            float ambientB = atof(mxmlElementGetAttr(ambientNode, "b"));

            // Diffuse
            mxml_node_t *diffuseNode = mxmlFindElement(lightNode, lightNode, "diffuse", NULL, NULL, MXML_DESCEND);
            float diffuseR = atof(mxmlElementGetAttr(diffuseNode, "r"));
            float diffuseG = atof(mxmlElementGetAttr(diffuseNode, "g"));
            float diffuseB = atof(mxmlElementGetAttr(diffuseNode, "b"));

            // Specular
            mxml_node_t *specularNode = mxmlFindElement(lightNode, lightNode, "specular", NULL, NULL, MXML_DESCEND);
            float specularR = atof(mxmlElementGetAttr(specularNode, "r"));
            float specularG = atof(mxmlElementGetAttr(specularNode, "g"));
            float specularB = atof(mxmlElementGetAttr(specularNode, "b"));

            GLenum lightID;
            if (strcmp(lightID_str, "GL_LIGHT0") == 0) lightID = GL_LIGHT0;
            else if (strcmp(lightID_str, "GL_LIGHT1") == 0) lightID = GL_LIGHT1;
            else {
                fprintf(stderr, "Unknown light ID '%s'.\n", lightID_str);
                continue;
            }

            ucncLight *light = ucncLightNew(lightID, posX, posY, posZ, ambientR, ambientG, ambientB, diffuseR, diffuseG, diffuseB, specularR, specularG, specularB);
            if (light) {
                ucncLight **temp = realloc(loadedLights, (loadedLightCount + 1) * sizeof(ucncLight*));
                if (!temp) {
                    fprintf(stderr, "Reallocation failed for lights.\n");
                    ucncLightFree(light);
                    freeLights(loadedLights, loadedLightCount);
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
    for (int i = 0; i < assemblyCount; i++) {
        ucncAssembly *current = assemblies[i];
        if (strcmp(current->parentName, "NULL") == 0) {
            *rootAssembly = current;
        } else {
            for (int j = 0; j < assemblyCount; j++) {
                if (strcmp(assemblies[j]->name, current->parentName) == 0) {
                    ucncAssemblyAddAssembly(assemblies[j], current);
                    break;
                }
            }
        }
    }

    if (*rootAssembly == NULL) {
        fprintf(stderr, "No root assembly found.\n");
        freeLights(loadedLights, loadedLightCount);
        for (int i = 0; i < assemblyCount; i++) {
            ucncAssemblyFree(assemblies[i]);
        }
        free(assemblies);
        return 0;
    }

    free(assemblies);  // Free the temporary assemblies array after setting the hierarchy
    return 1; // Success
}