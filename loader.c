// loader.c

#include "loader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define EXPECTED_FIELDS 17 // Update the expected number of fields

ucncActor* ucncLoadMachine(const char *configFilePath) {
    // Open the configuration file
    FILE *file = fopen(configFilePath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open config file %s\n", configFilePath);
        return NULL;
    }

    // Map to store actors by name
    typedef struct ActorNode {
        ucncActor *actor;
        struct ActorNode *next;
    } ActorNode;
    ActorNode *actorList = NULL;

    // Read and parse each line
    char line[1024];
    int lineNumber = 0;
    while (fgets(line, sizeof(line), file)) {
        lineNumber++;
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = '\0';

        // Tokenize the line
        char *tokens[16];
        int tokenCount = 0;
        char *token = strtok(line, "\t");
        while (token != NULL && tokenCount < 16) {
            tokens[tokenCount++] = token;
            token = strtok(NULL, "\t");
        }
        if (tokenCount < EXPECTED_FIELDS) {
            fprintf(stderr, "Incorrect number of fields in line %d\n", lineNumber);
            continue;
        }

        // Create and populate the actor
        ucncActor *actor = ucncActorCreate();
        if (!actor) {
            continue;
        }

        // Assign values
        actor->name = strdup(tokens[0]);
        const char *parentName = tokens[1];
        actor->stlFile = strdup(tokens[2]);
        actor->origin[0] = atof(tokens[3]);
        actor->origin[1] = atof(tokens[4]);
        actor->origin[2] = atof(tokens[5]);
        actor->position[0] = atof(tokens[6]);
        actor->position[1] = atof(tokens[7]);
        actor->position[2] = atof(tokens[8]);
        actor->rotation[0] = atof(tokens[9]);
        actor->rotation[1] = atof(tokens[10]);
        actor->rotation[2] = atof(tokens[11]);
        actor->color[0] = atof(tokens[12]);
        actor->color[1] = atof(tokens[13]);
        actor->color[2] = atof(tokens[14]);

        // Read IsAxis and AxisName
        actor->isAxis = atoi(tokens[15]);
        if (actor->isAxis) {
            actor->axisName = strdup(tokens[16]);
        } else {
            actor->axisName = NULL;
        }

        // Load STL file
        if (!ucncActorLoadSTL(actor)) {
            fprintf(stderr, "Failed to load STL file '%s' for actor '%s'\n", actor->stlFile, actor->name);
            ucncActorFree(actor);
            continue;
        }

        // Add actor to the list
        ActorNode *node = malloc(sizeof(ActorNode));
        if (!node) {
            fprintf(stderr, "Memory allocation failed\n");
            ucncActorFree(actor);
            continue;
        }
        node->actor = actor;
        node->next = actorList;
        actorList = node;

        // Store parent name for later linkage
        actor->parent = (ucncActor *)strdup(parentName);
    }

    fclose(file);

    // Establish parent-child relationships
    ActorNode *current = actorList;
    while (current) {
        if (current->actor->parent && strcmp((char *)current->actor->parent, "NULL") != 0 && strlen((char *)current->actor->parent) > 0) {
            // Find parent actor
            ucncActor *parentActor = NULL;
            ActorNode *search = actorList;
            while (search) {
                if (strcmp(search->actor->name, (char *)current->actor->parent) == 0) {
                    parentActor = search->actor;
                    break;
                }
                search = search->next;
            }
            if (parentActor) {
                free((char *)current->actor->parent); // Free the temporary parent name string
                current->actor->parent = parentActor;
                // Add to parent's children
                parentActor->children = realloc(parentActor->children, (parentActor->childCount + 1) * sizeof(ucncActor *));
                if (!parentActor->children) {
                    fprintf(stderr, "Memory allocation failed for children array\n");
                    // Handle memory allocation failure
                } else {
                    parentActor->children[parentActor->childCount] = current->actor;
                    parentActor->childCount++;
                }
            } else {
                fprintf(stderr, "Parent actor '%s' not found for actor '%s'\n", (char *)current->actor->parent, current->actor->name);
                free((char *)current->actor->parent); // Free the temporary parent name string
                current->actor->parent = NULL;
            }
        } else {
            // No parent; this is a root actor
            free((char *)current->actor->parent); // Free the temporary parent name string
            current->actor->parent = NULL;
        }
        current = current->next;
    }

    // Find root actor(s)
    ucncActor *rootActor = NULL;
    current = actorList;
    while (current) {
        if (!current->actor->parent) {
            rootActor = current->actor;
            break; // Assuming single root actor
        }
        current = current->next;
    }

    // Clean up temporary list but keep actors
    while (actorList) {
        ActorNode *next = actorList->next;
        free(actorList);
        actorList = next;
    }

    return rootActor;
}
