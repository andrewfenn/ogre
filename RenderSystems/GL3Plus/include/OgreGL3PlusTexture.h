/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __GL3PlusTexture_H__
#define __GL3PlusTexture_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreGL3PlusSupport.h"
#include "OgrePlatform.h"
#include "OgreRenderTexture.h"
#include "OgreTexture.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre {
    class _OgreGL3PlusExport GL3PlusTexture : public Texture
    {
        public:
            // Constructor
            GL3PlusTexture(ResourceManager* creator, const String& name, ResourceHandle handle,
                const String& group, bool isManual, ManualResourceLoader* loader, 
                GL3PlusSupport& support);

            virtual ~GL3PlusTexture();

            void createRenderTexture();
            /// @copydoc Texture::getBuffer
            HardwarePixelBufferSharedPtr getBuffer(size_t face, size_t mipmap);

            // Takes the OGRE texture type (1d/2d/3d/cube) and returns the appropriate GL one
            GLenum getGL3PlusTextureTarget(void) const;

			/** Returns the GL Id of the texture. When this texture is a render target
				with FSAA enabled and explicit resolves, it may return the ID of the
				Multisample texture version instead. When this happens, outIsFsaa
				will be set to true.
			@param outIsFsaa
				True if the returned value belongs to a 2D multisample buffer. This
				will happen if all of the following conditions are met:
					* Texture is a RenderTarget that uses antialising
					* Explicit resolves are turned on
					* The texture hasn't been resolved yet.
			@return
				The GLuint handle/id of the texture.
			*/
            GLuint getGLID( bool &outIsFsaa ) const
            {
				GLuint retVal = mTextureID;
				bool isFsaa = false;

				if( mFSAA > 0 )
				{
					RenderTarget *renderTarget = mSurfaceList[0]->getRenderTarget();
					if( !mFsaaExplicitResolve )
					{
						for( size_t face=0; face<getNumFaces(); ++face )
						{
							renderTarget = mSurfaceList[face * (mNumMipmaps+1)]->getRenderTarget();
							if( renderTarget->isFsaaResolveDirty() )
								renderTarget->swapBuffers( false );
						}
					}
					else if( renderTarget->isFsaaResolveDirty() )
					{
						//GL 3.3+ supports explicit resolves. Only use the
						//Fsaa texture before it has been resolved
						renderTarget->getCustomAttribute( "GL_MULTISAMPLEFBOID", &retVal );
						isFsaa = true;
					}
				}

				outIsFsaa = isFsaa;
                return mTextureID;
            }

		void getCustomAttribute(const String& name, void* pData);

        protected:
            /// @copydoc Texture::createInternalResourcesImpl
            void createInternalResourcesImpl(void);
            /// @copydoc Resource::prepareImpl
            void prepareImpl(void);
            /// @copydoc Resource::unprepareImpl
            void unprepareImpl(void);
            /// @copydoc Resource::loadImpl
            void loadImpl(void);
            /// @copydoc Resource::freeInternalResourcesImpl
            void freeInternalResourcesImpl(void);

		/** internal method, create GL3PlusHardwarePixelBuffers for every face and
             mipmap level. This method must be called after the GL texture object was created,
             the number of mipmaps was set (GL_TEXTURE_MAX_LEVEL) and glTexImageXD was called to
             actually allocate the buffer
             */
            void _createSurfaceList();

            /// Used to hold images between calls to prepare and load.
            typedef SharedPtr<vector<Image>::type > LoadedImages;

            /** Vector of images that were pulled from disk by
             prepareLoad but have yet to be pushed into texture memory
             by loadImpl.  Images should be deleted by loadImpl and unprepareImpl.
             */
            LoadedImages mLoadedImages;


        private:
            GLuint mTextureID;
            GL3PlusSupport& mGLSupport;

            /// Vector of pointers to subsurfaces
            typedef vector<HardwarePixelBufferSharedPtr>::type SurfaceList;
            SurfaceList mSurfaceList;

    };

    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to GL3PlusTexturePtr
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass,
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class _OgreGL3PlusExport GL3PlusTexturePtr : public SharedPtr<GL3PlusTexture>
    {
        public:
            GL3PlusTexturePtr() : SharedPtr<GL3PlusTexture>() {}
            explicit GL3PlusTexturePtr(GL3PlusTexture* rep) : SharedPtr<GL3PlusTexture>(rep) {}
            GL3PlusTexturePtr(const GL3PlusTexturePtr& r) : SharedPtr<GL3PlusTexture>(r) {}

            GL3PlusTexturePtr(const ResourcePtr& r) : SharedPtr<GL3PlusTexture>()
            {
                // lock & copy other mutex pointer
                OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
                {
                    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
                    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                    pRep = static_cast<GL3PlusTexture*>(r.getPointer());
                    pUseCount = r.useCountPointer();
                    if (pUseCount)
                    {
                        ++(*pUseCount);
                    }
                }
            }

            GL3PlusTexturePtr(const TexturePtr& r) : SharedPtr<GL3PlusTexture>()
            {
                *this = r;
            }

        /// Operator used to convert a ResourcePtr to a GL3PlusTexturePtr
            GL3PlusTexturePtr& operator=(const ResourcePtr& r)
            {
                if (pRep == static_cast<GL3PlusTexture*>(r.getPointer()))
                {
                    return *this;
                }
                release();
                // lock & copy other mutex pointer
                OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
                {
                    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
                    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                    pRep = static_cast<GL3PlusTexture*>(r.getPointer());
                    pUseCount = r.useCountPointer();
                    if (pUseCount)
                    {
                        ++(*pUseCount);
                    }
                }
                else
                {
                    // RHS must be a null pointer
                    assert(r.isNull() && "RHS must be null if it has no mutex!");
                    setNull();
                }
                return *this;
            }

            /// Operator used to convert a TexturePtr to a GL3PlusTexturePtr
            GL3PlusTexturePtr& operator=(const TexturePtr& r)
            {
                if (pRep == static_cast<GL3PlusTexture*>(r.getPointer()))
                    return *this;
                release();
                // lock & copy other mutex pointer
                OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
                {
                    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
                    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                    pRep = static_cast<GL3PlusTexture*>(r.getPointer());
                    pUseCount = r.useCountPointer();
                    if (pUseCount)
                    {
                        ++(*pUseCount);
                    }
                }
                else
                {
                    // RHS must be a null pointer
                    assert(r.isNull() && "RHS must be null if it has no mutex!");
                    setNull();
                }
                return *this;
            }
    };
}

#endif
