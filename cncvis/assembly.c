/* assembly.c */

#include "assembly.h"

// Implementation of ucncAssemblyNew, ucncAssemblyAddActor, ucncAssemblyAddAssembly, ucncAssemblyRender, ucncAssemblyFree

ucncAssembly *ucncAssemblyNew(const char *name, const char *parentName,
                              float originX, float originY, float originZ,
                              float positionX, float positionY, float positionZ,
                              float rotationX, float rotationY, float rotationZ,
                              float homePositionX, float homePositionY, float homePositionZ,
                              float homeRotationX, float homeRotationY, float homeRotationZ,
                              float colorR, float colorG, float colorB,
                              const char *motionType, char motionAxis, int invertMotion)
{
    // Allocate memory for the assembly
    ucncAssembly *assembly = malloc(sizeof(ucncAssembly));
    if (!assembly)
    {
        fprintf(stderr, "Memory allocation failed for ucncAssembly '%s'.\n", name ? name : "unknown");
        return NULL;
    }

    // Set the name (mandatory)
    if (name && strlen(name) > 0)
    {
        snprintf(assembly->name, sizeof(assembly->name), "%s", name);
    }
    else
    {
        fprintf(stderr, "Invalid assembly name (NULL or empty).\n");
        free(assembly);
        return NULL;
    }

    // Set the parent name, defaulting to "NULL" if parentName is NULL
    if (parentName && strlen(parentName) > 0)
    {
        snprintf(assembly->parentName, sizeof(assembly->parentName), "%s", parentName);
    }
    else
    {
        snprintf(assembly->parentName, sizeof(assembly->parentName), "NULL");
    }

    // Set origin, position, rotation, and color
    assembly->originX = originX;
    assembly->originY = originY;
    assembly->originZ = originZ;
    assembly->positionX = positionX;
    assembly->positionY = positionY;
    assembly->positionZ = positionZ;
    assembly->rotationX = rotationX;
    assembly->rotationY = rotationY;
    assembly->rotationZ = rotationZ;

    // Set home position and rotation
    assembly->homePositionX = homePositionX;
    assembly->homePositionY = homePositionY;
    assembly->homePositionZ = homePositionZ;
    assembly->homeRotationX = homeRotationX;
    assembly->homeRotationY = homeRotationY;
    assembly->homeRotationZ = homeRotationZ;

    // Set color values
    assembly->colorR = colorR;
    assembly->colorG = colorG;
    assembly->colorB = colorB;

    // Initialize motion fields
    if (motionType && strlen(motionType) > 0)
    {
        snprintf(assembly->motionType, sizeof(assembly->motionType), "%s", motionType);
    }
    else
    {
        snprintf(assembly->motionType, sizeof(assembly->motionType), "none"); // Default to "none"
    }

    assembly->motionAxis = motionAxis;
    assembly->invertMotion = invertMotion;

    // Initialize actor and assembly lists (empty arrays)
    assembly->actors = NULL;
    assembly->actorCount = 0;
    assembly->assemblies = NULL;
    assembly->assemblyCount = 0;

    return assembly;
}



int ucncAssemblyAddActor(ucncAssembly *assembly, ucncActor *actor) {
    // Reallocate memory for the actor list
    ucncActor **temp = realloc(assembly->actors, (assembly->actorCount + 1) * sizeof(ucncActor*));
    if (!temp) {
        // If realloc fails, output error message and return failure
        fprintf(stderr, "Reallocation failed when adding actor '%s' to assembly '%s'.\n",
                actor->name[0] ? actor->name : "(unknown actor)",
                assembly->name[0] ? assembly->name : "(unknown assembly)");
        return 0; // Failure
    }

    // Assign the newly allocated memory to the actors array
    assembly->actors = temp;
    assembly->actors[assembly->actorCount] = actor;  // Add the new actor
    assembly->actorCount++;  // Increment the actor count

    return 1; // Success
}


int ucncAssemblyAddAssembly(ucncAssembly *parent, ucncAssembly *child)
{
    if (!parent || !child)
    {
        fprintf(stderr, "Invalid parameters: parent or child assembly is NULL.\n");
        return 0; // Failure
    }

    // Reallocate memory for the assembly list
    ucncAssembly **temp = realloc(parent->assemblies, (parent->assemblyCount + 1) * sizeof(ucncAssembly *));
    if (!temp)
    {
        // If realloc fails, output error message and return failure
        fprintf(stderr, "Reallocation failed when adding assembly '%s' to parent assembly '%s'.\n",
                child->name[0] ? child->name : "(unknown child assembly)",
                parent->name[0] ? parent->name : "(unknown parent assembly)");
        return 0; // Failure
    }

    // If realloc succeeds, update the parent->assemblies pointer
    parent->assemblies = temp;

    // Add the child assembly to the parent's assembly list
    parent->assemblies[parent->assemblyCount] = child;
    parent->assemblyCount++;

    return 1; // Success
}


void ucncAssemblyRender(const ucncAssembly *assembly) {


    glPushMatrix(); // Save the current transformation matrix

    // Apply assembly transformations
    // Translate to the assembly position in world space
    glTranslatef(assembly->positionX, assembly->positionY, assembly->positionZ);

    // Translate to the origin, then apply rotations
    glTranslatef(assembly->originX, assembly->originY, assembly->originZ);
    glRotatef(assembly->rotationX, 1.0f, 0.0f, 0.0f); // X-axis rotation
    glRotatef(assembly->rotationY, 0.0f, 1.0f, 0.0f); // Y-axis rotation
    glRotatef(assembly->rotationZ, 0.0f, 0.0f, 1.0f); // Z-axis rotation
    drawAxis(100.0f);
    // Translate back by the origin (undo translation)
    glTranslatef(-assembly->originX, -assembly->originY, -assembly->originZ);

    // Debugging output: Print assembly details
    // printf("Rendering assembly: %s at position (%.2f, %.2f, %.2f) with rotation (%.2f, %.2f, %.2f)\n",
    //       assembly->name, assembly->positionX, assembly->positionY, assembly->positionZ,
    //       assembly->rotationX, assembly->rotationY, assembly->rotationZ);

    // Render all actors in this assembly
    for (int i = 0; i < assembly->actorCount; i++) {
        ucncActorRender(assembly->actors[i]);
    }

    // Render all child assemblies recursively
    for (int i = 0; i < assembly->assemblyCount; i++) {
        ucncAssemblyRender(assembly->assemblies[i]);
    }

    glPopMatrix(); // Restore the previous transformation matrix
}



ucncAssembly *findAssemblyByName(ucncAssembly *rootAssembly, const char *name)
{
    if (!rootAssembly || !name)
        return NULL;

    printf("Checking assembly: %s\n", rootAssembly->name);

    // Check if the root assembly's name matches
    if (strcmp(rootAssembly->name, name) == 0)
    {
        printf("Found assembly: %s\n", rootAssembly->name);
        return rootAssembly;
    }

    // Recursively search child assemblies
    for (int i = 0; i < rootAssembly->assemblyCount; i++)
    {
        ucncAssembly *found = findAssemblyByName(rootAssembly->assemblies[i], name);
        if (found)
        {
            return found;
        }
    }

    return NULL; // Return NULL if not found
}

void ucncAssemblyFree(ucncAssembly *assembly)
{
    if (!assembly)
        return;

    // Free all actors
    for (int i = 0; i < assembly->actorCount; i++)
    {
        if (assembly->actors[i])
        {
            ucncActorFree(assembly->actors[i]); // Ensure actors are freed properly
        }
    }
    free(assembly->actors); // Free the array of actor pointers

    // Recursively free child assemblies
    for (int i = 0; i < assembly->assemblyCount; i++)
    {
        if (assembly->assemblies[i])
        {
            ucncAssemblyFree(assembly->assemblies[i]); // Recursive call to free child assemblies
        }
    }
    free(assembly->assemblies); // Free the array of child assembly pointers

    // Free the memory allocated for the assembly itself
    free(assembly);
}

void cleanupAssemblies(ucncAssembly **assemblies, int assemblyCount)
{
    for (int i = 0; i < assemblyCount; i++)
    {
        ucncAssemblyFree(assemblies[i]);
    }
    free(assemblies);
}
