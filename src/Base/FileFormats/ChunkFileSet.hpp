//
//  ChunkFileSet.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 15/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef ChunkFileSet_hpp
#define ChunkFileSet_hpp

#include <stdio.h>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

class TaskResults;

class ChunkFileSet {
 public:
  ChunkFileSet(std::string basePath, size_t nDatasets, size_t chunkId);

  ~ChunkFileSet();

  size_t GetId() { return id; };
  inline FILE* GetFile(size_t datasetId) { return files[datasetId]; };

  void SetMinChunkSize(size_t size) { minChunkSize = size; };

  size_t WriteToChunk(std::unique_ptr<TaskResults> const& results,
                      const std::vector<std::string>& orderName);
  bool IsChunkBig();
  void FlushData();

  std::vector<std::tuple<void*, size_t, size_t>> ReadEntry(
      size_t regOffset, bool lastItems = false);

 private:
  size_t WriteDataToChunk(size_t datasetId, const void* ptr, size_t dataSize);
  void ReadChunkRegister();
  void Initialise();

  std::vector<FILE*> files;
  size_t totalFilesSize = 0;
  std::vector<size_t> fileSizes;
  std::vector<size_t> fileOffsets;
  std::vector<std::string> fileNames;

  std::string basePath;
  std::string registerFileName;
  FILE* registerFile;
  size_t registerTrajSize;
  size_t N;

  size_t* buffer;
  size_t bufferByteSize;
  size_t id;
  size_t nTrajWritten = 0;
  bool initialised    = false;

  size_t minChunkSize = 1024 * 1024 * 1024;

  // Loaded data
  std::vector<size_t> registerData;
};

#endif /* ChunkFileSet_hpp */
