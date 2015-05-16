#include "ExternalSort.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <queue>
#include <unistd.h>

using namespace std;

// Struct for the k-merge-phase which
// associates a value with its merge buffer
struct MergeItem
{
    uint64_t value;
    uint64_t index;
    bool operator < (const MergeItem& a) const
    {
        return a.value < value;
    }
};

struct MergeBuffer
{
    FILE *file;
    vector<uint64_t> buffer;
    uint64_t index;
    uint64_t length;
    bool finished;
};

void externalSort (int fdInput, uint64_t size, int fdOutput, uint64_t memSize)
{
    if (size == 0) return;

    uint64_t count = memSize / sizeof(uint64_t);
    uint64_t chunks = (size + count-1) / count;
    uint64_t lastIteration = size - (count*(chunks-1));
    size_t bytesToRead = count * sizeof(uint64_t);

    vector<FILE*> chunkFiles;
    chunkFiles.reserve(count);

    vector<uint64_t> inputBuffer;
    inputBuffer.resize(count);

    // Read input file and store k sorted temporary files
    for (uint64_t i = 0; i < chunks; i++)
    {
        // The last chunk will probably not be completely full
        // Adjust the number of ints to read accordingly
        if ((i+1) == chunks)
        {
            count = lastIteration;
            inputBuffer.resize(count);
            bytesToRead = count * sizeof(uint64_t);
        }


        // Read the next chunk
        if (read(fdInput, &inputBuffer[0], bytesToRead) != (ssize_t) bytesToRead)
        {
            cerr << "Reading of input failed!" << endl;
            return;
        }

        // Sort the chunk using the std lib
        sort(inputBuffer.begin(), inputBuffer.end());

        // Write chunk to temporary file
        FILE* tmpf = tmpfile();
        chunkFiles.push_back(tmpf);
        fwrite(&inputBuffer[0], sizeof(uint64_t), count, tmpf);
    }

    inputBuffer.clear();
    inputBuffer.shrink_to_fit();

    // we need one buffer for each chunk plus 1 for buffering the output
    uint64_t mergeBufferSize = count / (chunks+1);

    // pq for the k-way merge
    priority_queue<MergeItem> mergePQ;

    // initialize merge buffers for each chunk
    vector<MergeBuffer*> mergeBuffers;
    mergeBuffers.resize(chunks);
    for (uint64_t i = 0; i < chunks; i++)
    {
        MergeBuffer *mergeBuffer = new MergeBuffer;
        mergeBuffers[i] = mergeBuffer;
        mergeBuffer->file = chunkFiles[i];
        mergeBuffer->index = 0;
        mergeBuffer->finished = false;

        // in case the last chunk is shorte than the merge buffer size
        if ((i+1) == chunks && mergeBufferSize > lastIteration)
        {
            mergeBuffer->buffer.resize(lastIteration);
            mergeBuffer->length = lastIteration;
        }
        else
        {
            mergeBuffer->buffer.resize(mergeBufferSize);
            mergeBuffer->length = mergeBufferSize;
        }

        // Go back to the beginning of the file and fill the buffer
        rewind(mergeBuffer->file);
        if (fread(&mergeBuffer->buffer[0], sizeof(uint64_t), mergeBufferSize, mergeBuffer->file)
                != mergeBufferSize)
        {
            cerr << "Reading from temporary chunk file " << i << " failed!" << endl;
        }

        // push the first element of the chunk onto the pq
        mergePQ.push( MergeItem{ mergeBuffer->buffer[0], i } );
        mergeBuffer->index++;
        mergeBuffer->length--;
    }

    uint64_t *outputBuffer = (uint64_t *) malloc(mergeBufferSize * sizeof(uint64_t));
    uint64_t outputIndex = 0;

    // As long as the merge is not complete
    while (!mergePQ.empty())
    {
        // get smallest element from queue and add to the buffer
        MergeItem min = mergePQ.top();
        mergePQ.pop();
        outputBuffer[outputIndex] = min.value;
        outputIndex++;

        // insert the next value of the chunk from which the value was taken
        // (unless the buffer is empty)
        MergeBuffer *sourceBuffer = mergeBuffers[min.index];
        if (sourceBuffer->length > 0)
        {
            mergePQ.push(MergeItem
            {
                sourceBuffer->buffer[sourceBuffer->index],
                min.index }
                        );
            sourceBuffer->index++;
            sourceBuffer->length--;

            // Buffer is empty, try to read more from disk
            if (sourceBuffer->length == 0 && !sourceBuffer->finished)
            {
                sourceBuffer->index = 0;
                sourceBuffer->length = fread(&sourceBuffer->buffer[0], sizeof(uint64_t),
                                             mergeBufferSize, sourceBuffer->file);
                // If the buffer couldn't be filled completely, the chunk has reached its end
                // Close the associated filestream
                if (sourceBuffer->length < mergeBufferSize)
                {
                    fclose(sourceBuffer->file);
                    sourceBuffer->finished = true;
                }
            }
        }

        // In case the output buffer is full, flush its content to the output file
        if (outputIndex == mergeBufferSize-1)
        {
            if (write(fdOutput, outputBuffer, outputIndex*sizeof(uint64_t))
                    != (ssize_t) (outputIndex*sizeof(uint64_t)))
            {
                cerr << "Writing to output file failed!" << endl;
            }
            outputIndex = 0;
        }
    }

    // Sorting is complete, flush the remaining elements from the buffer to the output file
    if (outputIndex > 0)
    {
        if (write(fdOutput, outputBuffer, outputIndex*sizeof(uint64_t))
                != (ssize_t) (outputIndex*sizeof(uint64_t)))
            cerr << "Writing to output file failed!" << endl;
    }
}



