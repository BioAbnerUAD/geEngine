/*****************************************************************************/
/**
 * @file    geD3D11HardwareBuffer.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2018/06/13
 * @brief   Common functionality for all DirectX 11 hardware buffers.
 *
 * Common functionality for all DirectX 11 hardware buffers.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/
#pragma once

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "gePrerequisitesD3D11.h"

#include <geHardwareBuffer.h>

namespace geEngineSDK {
  namespace geCoreThread {
    /**	Available types of DX11 buffers. */
    namespace BUFFER_TYPE {
      enum E {
        /**
         * Contains geometry vertices and their properties.
         */
        kVERTEX = 0x1,

        /**
         * Contains triangle to vertex mapping.
         */
        kINDEX = 0x2,

        /**
         * Contains GPU program parameters.
         */
        kCONSTANT = 0x4,

        /**
         * Special value signifying a buffer is of generic type.
         * Not an actual buffer.
         */
        kGROUP_GENERIC = 0x8,

        /**
         * Generic buffer that holds one or more user-defined structures laid
         * out sequentially.
         */
        kSTRUCTURED = kGROUP_GENERIC | 0x10,

        /**
         * Generic buffer that holds raw block of bytes with no defined
         * structure.
         */
        kRAW = kGROUP_GENERIC | 0x20,

        /**
         * Generic buffer that is used for holding parameters used for indirect
         * rendering.
         */
        kINDIRECTARGUMENT = kGROUP_GENERIC | 0x40,

        /**
         * Generic buffer that allows the GPU program to use append/consume
         * functionality.
         */
        kAPPENDCONSUME = kGROUP_GENERIC | 0x80,

        /**
         * Generic buffer that contains primitive types.
         */
        kSTANDARD = kGROUP_GENERIC | 0x100
      };
    }

    class D3D11HardwareBuffer : public HardwareBuffer
    {
     public:
      D3D11HardwareBuffer(BUFFER_TYPE::E btype,
                          GPU_BUFFER_USAGE::E usage,
                          uint32 elementCount,
                          uint32 elementSize,
                          D3D11Device& device,
                          bool systemMemory = false,
                          bool streamOut = false,
                          bool randomGPUWrite = false,
                          bool useCounter = false);

      ~D3D11HardwareBuffer();

      /**
       * @copydoc HardwareBuffer::readData
       */
      void
      readData(uint32 offset,
               uint32 length,
               void* dest,
               uint32 deviceIdx = 0,
               uint32 queueIdx = 0) override;

      /**
       * @copydoc HardwareBuffer::writeData
       */
      void
      writeData(uint32 offset,
                uint32 length,
                const void* source,
                BUFFER_WRITE_TYPE::E writeFlags = BUFFER_WRITE_TYPE::kNORMAL,
                uint32 queueIdx = 0) override;

      /**
       * @copydoc HardwareBuffer::copyData
       */
      void
      copyData(HardwareBuffer& srcBuffer,
               uint32 srcOffset,
               uint32 dstOffset,
               uint32 length,
               bool discardWholeBuffer = false,
               const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

      /**
       * @brief  Returns the internal DX11 buffer object.
       */
      ID3D11Buffer*
      getD3DBuffer() const {
        return m_d3dBuffer;
      }

     protected:
      /**
       * @copydoc HardwareBuffer::map
       */
      void*
      map(uint32 offset,
          uint32 length,
          GPU_LOCK_OPTIONS::E options,
          uint32 deviceIdx,
          uint32 queueIdx) override;

      /**
       * @copydoc HardwareBuffer::unmap
       */
      void
      unmap() override;

      BUFFER_TYPE::E m_bufferType;
      bool m_randomGPUWrite;
      bool m_useCounter;
      uint32 m_elementCount;
      uint32 m_elementSize;
      GPU_BUFFER_USAGE::E m_usage;

      ID3D11Buffer* m_d3dBuffer;

      bool m_useTempStagingBuffer;
      D3D11HardwareBuffer* m_pTempStagingBuffer;
      bool m_stagingUploadNeeded;

      D3D11Device& m_device;
      D3D11_BUFFER_DESC m_desc;
    };
  }
}
