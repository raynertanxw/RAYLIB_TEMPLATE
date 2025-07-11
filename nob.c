#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NOB_IMPLEMENTATION
#include "nob.h" // Code from: https://github.com/tsoding/nob.h

#define STB_RECT_PACK_IMPLEMENTATION
#include "nob_src/stb_rect_pack.h"
#define STB_IMAGE_IMPLEMENTATION
#include "nob_src/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "nob_src/stb_image_write.h"

#include "src/build_defines.h"
#include "nob_src/raylib_extracts.h"

#ifdef _WIN32
#define MAX_PATH_LENGTH MAX_PATH
#else
#define MAX_PATH_LENGTH PATH_MAX
#endif // _WIN32

#define SRC_FOLDER "src/"
#define RESOURCE_FOLDER "resources/"
#define BUILD_FOLDER "bin/"
#define OUT_PATH BUILD_FOLDER "my-raylib-game" // TEMPLATE: Rename to project name

// ::HASH FUNCTIONS
static unsigned int hash_string(const char* str) {
  if (!str) return 0;
  return ComputeCRC32((unsigned char*)str, strlen(str));
}

static unsigned int hash_file(const char* filename) {
  if (!filename) return 0; // Return 0 if filename is NULL

  FILE* file = fopen(filename, "rb"); // Open the file in binary mode
  if (!file) {
    perror("Error opening file");
    return 0;
  }

  unsigned int  hash = 0;
  unsigned char buffer[1024]; // Buffer to hold chunks of file data
  size_t        bytesRead;

  // Read the file in chunks and update the hash
  while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    hash = ComputeCRC32(buffer, bytesRead);
  }

  fclose(file);
  return hash;
}

static uint32_t hash_img_directory(const char* directory_path) {
  Nob_File_Paths filenames = {0};
  if (!nob_read_entire_dir(directory_path, &filenames)) exit(1);

  uint32_t combined_hash = 0;
  for (unsigned int i = 0; i < filenames.count; i++) {
    const char* filename = filenames.items[i];
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) continue; // Ignore parent and self
    if (strncmp(filename, ".", 1) == 0) continue;                            // Ignore hidden files.

    char filepath[MAX_PATH_LENGTH];
    snprintf(filepath, sizeof(filepath), "%s%s", directory_path, filename);
    Nob_File_Type filetype = nob_get_file_type(filepath);

    if (filetype == NOB_FILE_REGULAR) {
      const char* fileExtension = GetFileExtension(filepath);
      if (fileExtension && (strcmp(fileExtension, ".png") == 0 ||
                               strcmp(fileExtension, ".jpg") == 0 ||
                               strcmp(fileExtension, ".bmp") == 0)) {

        uint32_t filename_hash = hash_string(GetFileName(filepath));
        uint32_t file_hash = hash_file(filepath);
        combined_hash ^= (filename_hash ^ file_hash);
      }
    }
  }

  return combined_hash;
}

// ::SPRITE PACKER HELPERS
typedef struct {
  int x, y, width, height;
} Rect;

typedef struct {
  const char* filename;
  Rect        rect;
} NOB_SpriteMetadata;

typedef struct {
  unsigned char* data;
  int            width, height, channels;
} ImageBuffer;

ImageBuffer CreateBlankImage(int width, int height, int channels) {
  ImageBuffer img;
  img.width = width;
  img.height = height;
  img.channels = channels;
  img.data = (unsigned char*)calloc(width * height * channels, sizeof(unsigned char));
  return img;
}

ImageBuffer LoadImageBuffer(const char* filename) {
  ImageBuffer img;
  img.data = stbi_load(filename, &img.width, &img.height, &img.channels, 4); // Force 4 channels (RGBA)
  if (!img.data) {
    fprintf(stderr, "Failed to load image: %s\n", filename);
    exit(1);
  }
  return img;
}

void DrawImage(ImageBuffer* dst, ImageBuffer* src, Rect dstRect) {
  for (int y = 0; y < src->height; y++) {
    for (int x = 0; x < src->width; x++) {
      int dstX = dstRect.x + x;
      int dstY = dstRect.y + y;
      if (dstX < 0 || dstX >= dst->width || dstY < 0 || dstY >= dst->height) continue; // Bounds check

      int dstIndex = (dstY * dst->width + dstX) * dst->channels;
      int srcIndex = (y * src->width + x) * src->channels;

      memcpy(&dst->data[dstIndex], &src->data[srcIndex], src->channels);
    }
  }
}

void SaveImageBuffer(ImageBuffer* img, const char* filename) {
  if (!stbi_write_png(filename, img->width, img->height, img->channels, img->data, img->width * img->channels)) {
    fprintf(stderr, "Failed to save image: %s\n", filename);
    exit(1);
  }
}

void FreeImageBuffer(ImageBuffer* img) {
  if (img->data) {
    stbi_image_free(img->data);
    img->data = NULL;
  }
}

bool check_should_build_atlas_and_compute_hashes(const char* atlas_img_filename, const char* metadata_filename, const char* atlas_src_folder) {
  char imageFilePath[MAX_PATH_LENGTH];
  snprintf(imageFilePath, sizeof(imageFilePath), "%s%s", RESOURCE_FOLDER, atlas_img_filename);
  char metadataFilePath[MAX_PATH_LENGTH];
  snprintf(metadataFilePath, sizeof(metadataFilePath), "%s%s", RESOURCE_FOLDER, metadata_filename);
  char hashfileFilename[MAX_PATH_LENGTH];
  snprintf(hashfileFilename, MAX_PATH_LENGTH, "%s.hash", GetFileNameWithoutExt(atlas_img_filename));
  char hashFilePath[MAX_PATH_LENGTH];
  snprintf(hashFilePath, sizeof(hashFilePath), "%s%s", RESOURCE_FOLDER, hashfileFilename);
  char pathToAtlasSourceFolder[MAX_PATH_LENGTH];
  snprintf(pathToAtlasSourceFolder, sizeof(pathToAtlasSourceFolder), "%s%s/", RESOURCE_FOLDER, atlas_src_folder);

  bool     shouldRebuildAndComputeHash = false;
  uint32_t computedHash = 0;

  if (nob_file_exists(imageFilePath) < 1 ||
      nob_file_exists(metadataFilePath) < 1 ||
      nob_file_exists(hashFilePath) < 1) {
    nob_log(NOB_INFO, "Image/Metadata/Hash file not found. Triggering building of texture atlas for resources in folder: %s", atlas_src_folder);
    shouldRebuildAndComputeHash = true;
  } else {
    FILE* file = fopen(hashFilePath, "r");
    if (!file) {
      nob_log(NOB_INFO, "Unexpected error. Triggering building of texture atlas for resources in folder: %s", atlas_src_folder);
      shouldRebuildAndComputeHash = true;
    } else {
      uint32_t savedHash;
      if (fscanf(file, "%u", &savedHash) != 1) {
        nob_log(NOB_INFO, "Failed to read hash. Triggering rebuild for folder: %s", atlas_src_folder);
        shouldRebuildAndComputeHash = true;
      } else {
        computedHash = hash_img_directory(pathToAtlasSourceFolder);
        shouldRebuildAndComputeHash = savedHash != computedHash;
        if (shouldRebuildAndComputeHash) nob_log(NOB_INFO, "Hash mismatch. Triggering rebuild for folder: %s", atlas_src_folder);
      }
      fclose(file);
    }
  }

  if (shouldRebuildAndComputeHash) {
    if (computedHash == 0) computedHash = hash_img_directory(pathToAtlasSourceFolder);
    FILE* file = fopen(hashFilePath, "w");
    if (file) {
      fprintf(file, "%u\n", computedHash);
      fclose(file);
      nob_log(NOB_INFO, "Saved new hash for folder: %s", atlas_src_folder);
    } else {
      nob_log(NOB_INFO, "Failed to save hash file for folder: %s", atlas_src_folder);
    }
  }

  return shouldRebuildAndComputeHash;
}

int load_sprites_from_directory(const char* directory_path, NOB_SpriteMetadata* sprites, const unsigned int MAX_SPRITES) {
  int file_count = 0;

  Nob_File_Paths filenames = {0};
  if (!nob_read_entire_dir(directory_path, &filenames)) exit(1);
  for (size_t i = 0; i < filenames.count; i++) {
    if (file_count > MAX_SPRITES) {
      nob_log(NOB_ERROR, "Number of tile paths exceeds maxSprites.");
      exit(1);
    }

    const char* filename = filenames.items[i];
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) continue; // Ignore parent and self
    if (strncmp(filename, ".", 1) == 0) continue;                            // Ignore hidden files.
    const char* file_ext = GetFileExtension(filename);
    if (file_ext && (strcmp(file_ext, ".png") == 0 ||
                        strcmp(file_ext, ".jpg") == 0 ||
                        strcmp(file_ext, ".bmp") == 0)) {
      const size_t size_filepath = MAX_PATH_LENGTH * sizeof(char);
      char*        filepath = (char*)calloc(1, size_filepath);
      snprintf(filepath, size_filepath, "%s/%s", directory_path, filename);
      Nob_File_Type filetype = nob_get_file_type(filepath);
      if (filetype != NOB_FILE_REGULAR) continue;

      sprites[file_count].filename = filepath; // Store full path
      int channels;                            // Unused info.
      if (!stbi_info(filepath, &sprites[file_count].rect.width, &sprites[file_count].rect.height, &channels)) {
        nob_log(NOB_ERROR, "Failed to read sprite info: %s", filepath);
        exit(1);
      }

      file_count++;
    }
  }

  nob_log(NOB_INFO, "%d sprites loaded from directory: %s", file_count, directory_path);
  return file_count;
}

void save_atlas_metadata(const char* filename, NOB_SpriteMetadata* sprites, int count) {
  char path[MAX_PATH_LENGTH];
  snprintf(path, sizeof(path), "%s%s", RESOURCE_FOLDER, filename);

  FILE* file = fopen(path, "w");
  if (!file) {
    nob_log(NOB_ERROR, "Failed to write metadata file: %s", path);
    return;
  }

  fprintf(file, "%d\n", count); // Write the count of sprites
  for (int i = 0; i < count; i++) {
    fprintf(file, "%s %d %d %d %d\n",
        GetFileNameWithoutExt(sprites[i].filename), // Stripped filename
        sprites[i].rect.x,
        sprites[i].rect.y,
        sprites[i].rect.width,
        sprites[i].rect.height);
  }
  fclose(file);

  nob_log(NOB_INFO, "Successfully saved atlas meta file: %s", path);
}

void build_texture_atlas(const char* atlas_img_filename, const char* atlas_metadata_filename, const char* atlas_src_folder, int atlas_width, int atlas_height, const int MAX_SPRITES) {
  bool shouldBuild = check_should_build_atlas_and_compute_hashes(atlas_img_filename, atlas_metadata_filename, atlas_src_folder);
  if (!shouldBuild) {
    nob_log(NOB_INFO, "No changes detected for atlas path: %s, skipping build", atlas_src_folder);
    return;
  }
  char pathToAtlasFolder[MAX_PATH_LENGTH];
  snprintf(pathToAtlasFolder, sizeof(pathToAtlasFolder), "%s%s", RESOURCE_FOLDER, atlas_src_folder);
  nob_log(NOB_INFO, "Attempting to build texture atlas from path: %s", pathToAtlasFolder);

  // Start of building process.
  NOB_SpriteMetadata sprites[MAX_SPRITES];
  memset(sprites, 0, sizeof(sprites));
  int spriteCount = load_sprites_from_directory(pathToAtlasFolder, sprites, MAX_SPRITES);
  if (spriteCount == 0) {
    nob_log(NOB_ERROR, "No valid images found in the directory: %s", pathToAtlasFolder);
    exit(1);
  }

  stbrp_context context;
  stbrp_node    nodes[256];

  stbrp_init_target(&context, atlas_width, atlas_height, nodes, 256);

  stbrp_rect rects[MAX_SPRITES];
  for (int i = 0; i < spriteCount; i++) {
    rects[i].id = i;
    rects[i].w = sprites[i].rect.width;
    rects[i].h = sprites[i].rect.height;
  }

  if (stbrp_pack_rects(&context, rects, spriteCount)) {
    for (int i = 0; i < spriteCount; i++) {
      sprites[rects[i].id].rect = (Rect){
          rects[i].x,
          rects[i].y,
          rects[i].w,
          rects[i].h,
      };
    }

    ImageBuffer atlasImage = CreateBlankImage(atlas_width, atlas_height, 4);

    for (int i = 0; i < spriteCount; i++) {
      NOB_SpriteMetadata sprite = sprites[i];
      ImageBuffer        spriteImage = LoadImageBuffer(sprite.filename);
      DrawImage(&atlasImage, &spriteImage, sprite.rect);
      FreeImageBuffer(&spriteImage);
    }

    char atlasPath[MAX_PATH_LENGTH];
    snprintf(atlasPath, sizeof(atlasPath), "%s%s", RESOURCE_FOLDER, atlas_img_filename);
    SaveImageBuffer(&atlasImage, atlasPath);
    FreeImageBuffer(&atlasImage);
    nob_log(NOB_INFO, "Successfully saved atlas image: %s", atlasPath);

    // Save metadata
    save_atlas_metadata(atlas_metadata_filename, sprites, spriteCount);
  } else {
    nob_log(NOB_ERROR, "Failed to pack textures!");
    exit(1);
  }
}

// ::GENERAL HELPERS
bool delete_folder_recursive(const char* directory_path) {
  nob_log(NOB_INFO, "Delete Folder Recursive directory_path: %s", directory_path);
  if (nob_get_file_type(directory_path) != NOB_FILE_DIRECTORY) return false;
  Nob_File_Paths filenames = {0};
  if (!nob_read_entire_dir(directory_path, &filenames)) exit(1);

  for (size_t i = 0; i < filenames.count; i++) {
    const char* filename = filenames.items[i];
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) continue; // Ignore parent and self

    char filepath[MAX_PATH_LENGTH];
    snprintf(filepath, sizeof(filepath), "%s/%s", directory_path, filename);
    Nob_File_Type filetype = nob_get_file_type(filepath);
    if (filetype == NOB_FILE_DIRECTORY) {
      if (!delete_folder_recursive(filepath)) exit(1);
    } else {
      nob_delete_file(filepath);
    }
  }

  if (rmdir(directory_path) != 0) exit(1);
  return true;
}

// ::COMMANDS
void execute_cmd_sprite_packer() {
  build_texture_atlas(
      MAIN_ATLAS_IMAGE_FILE,
      MAIN_ATLAS_META_FILE,
      "mainAtlas",
      MAIN_ATLAS_SIZE,
      MAIN_ATLAS_SIZE,
      MAIN_ATLAS_MAX_SPRITES);

  // TEMPLATE: Build additional sprite atlas as defined in src/build_defines

  nob_log(NOB_INFO, "Sprite packer stage complete.");
}

void execute_cmd_clean() {
  Nob_File_Paths filenames = {0};
  if (!nob_read_entire_dir(BUILD_FOLDER, &filenames)) {
    nob_log(NOB_INFO, "Build Folder %s not found. Ignore this if you are running nob for first time on new machine, other commands will create the build folder.\n", BUILD_FOLDER);
  }

  for (size_t i = 0; i < filenames.count; i++) {
    const char* filename = filenames.items[i];
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) continue; // Ignore parent and self
    if (strncmp(filename, "logs", 4) == 0) continue;                         // Ignore log folder.
    if (strncmp(filename, "player", 6) == 0) continue;                       // Ignore player folder.

    char filepath[MAX_PATH_LENGTH];
    snprintf(filepath, sizeof(filepath), "%s%s", BUILD_FOLDER, filename);
    Nob_File_Type filetype = nob_get_file_type(filepath);

    if (filetype == NOB_FILE_DIRECTORY) {
      if (!delete_folder_recursive(filepath)) exit(1);
    } else {
      nob_delete_file(filepath);
    }
  }

  nob_log(NOB_INFO, "Successfully cleaned build folder.");
}

void execute_cmd_build(bool is_release) {
  Nob_Cmd build_cmd = {0};
  if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) exit(1);
  nob_cmd_append(&build_cmd, "cc", "-Wall", "-Wextra");
  if (is_release) {
    // TODO: Investigate what would be good here... maybe -O2?
    nob_cmd_append(&build_cmd, "-DDEBUG=0", "-O3", "-s"); // DEBUG FLAGS
  } else {
    nob_cmd_append(&build_cmd, "-DDEBUG=1", "-O0", "-g"); // DEBUG FLAGS
  }
  nob_cmd_append(&build_cmd, SRC_FOLDER "main.c");
#ifdef _WIN32
  nob_cmd_append(&build_cmd, "-L", "lib_win/", "-lraylib");
  nob_cmd_append(&build_cmd, "-lgdi32", "-lwinmm");
#else
  nob_cmd_append(&build_cmd, "-L", "lib_mac/", "-lraylib");
  nob_cmd_append(&build_cmd, "-framework", "CoreVideo", "-framework", "IOKit", "-framework", "Cocoa", "-framework", "GLUT", "-framework", "OpenGL");
#endif
  nob_cmd_append(&build_cmd, "-o", OUT_PATH);
  if (!nob_cmd_run_sync(build_cmd)) exit(1);

  nob_log(NOB_INFO, "Build complete!");
}

void execute_cmd_copy_resources() {
  Nob_File_Paths filenames = {0};
  if (!nob_read_entire_dir(RESOURCE_FOLDER, &filenames)) exit(1);

  char dest_folder[MAX_PATH_LENGTH];
  snprintf(dest_folder, sizeof(dest_folder), "%s%s", BUILD_FOLDER, RESOURCE_FOLDER);
  if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) exit(1);
  if (!nob_mkdir_if_not_exists(dest_folder)) exit(1);

  for (size_t i = 0; i < filenames.count; i++) {
    const char* filename = filenames.items[i];
    if (strncmp(filename, ".", 1) == 0) continue; // Ignore anything that begins with '.'
    // TODO: Copy the other resources in directory such as future sound folders.
    const char* file_ext = GetFileExtension(filename);
    if (file_ext == NULL) continue;
    if (strcmp(file_ext, ".hash") == 0) continue;

    char filepath[MAX_PATH_LENGTH];
    snprintf(filepath, sizeof(filepath), "%s%s", RESOURCE_FOLDER, filename);
    Nob_File_Type filetype = nob_get_file_type(filepath);
    if (filetype != NOB_FILE_REGULAR) continue;

    char dest_path[MAX_PATH_LENGTH];
    snprintf(dest_path, sizeof(dest_path), "%s%s", dest_folder, filename);
    nob_copy_file(filepath, dest_path); // Already logs at INFO level.
  }

  nob_log(NOB_INFO, "Resources copied to build folder.");
}

void execute_cmd_run() {
  Nob_Cmd run_cmd = {0};
  nob_cmd_append(&run_cmd, "./" OUT_PATH);
  if (!nob_cmd_run_sync(run_cmd)) exit(1);
}

typedef enum {
  NOB_BUILD_NORMAL,
  NOB_BUILD_RELEASE,
  NOB_CLEAN_ONLY,
  NOB_SPRITE_PACK_ONLY,
} BUILD_MODE;
int main(int argc, char** argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  BUILD_MODE mode = NOB_BUILD_NORMAL;

  if (argc > 1) {
    const char* build_param = argv[1];
    if (strcmp(build_param, "release") == 0) mode = NOB_BUILD_RELEASE;
    else if (strcmp(build_param, "clean") == 0) mode = NOB_CLEAN_ONLY;
    else if (strcmp(build_param, "spritepack") == 0) mode = NOB_SPRITE_PACK_ONLY;
    else {
      nob_log(NOB_ERROR, "Unrecognised build param: %s", build_param);
      exit(1);
    }
  }

  switch (mode) {
    case NOB_BUILD_NORMAL: {
      execute_cmd_clean();
      execute_cmd_sprite_packer();
      execute_cmd_build(false);
      execute_cmd_copy_resources();
      execute_cmd_run();
      break;
    }
    case NOB_BUILD_RELEASE: {
      execute_cmd_clean();
      execute_cmd_sprite_packer();
      execute_cmd_build(true);
      execute_cmd_copy_resources();
      break;
    }
    case NOB_CLEAN_ONLY: {
      execute_cmd_clean();
      break;
    }
    case NOB_SPRITE_PACK_ONLY: {
      execute_cmd_sprite_packer();
      break;
    }
    default: {
      nob_log(NOB_ERROR, "Unhanaled build mode: %d", mode);
      exit(1);
    }
  }

  return 0;
}
