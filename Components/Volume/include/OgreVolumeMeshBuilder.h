/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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
#ifndef __Ogre_Volume_MeshBuilder_H__
#define __Ogre_Volume_MeshBuilder_H__

#include <vector>
#include "OgreManualObject.h"
#include "OgreRenderOperation.h"
#include "OgreVector3.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSceneManager.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** Leightweight struct to represent a mesh vertex.
    */
    typedef struct _OgreVolumeExport Vertex
    {
        /// X coordinate of the position
        Real x;
        
        /// Y coordinate of the position
        Real y;
        
        /// Z coordinate of the position
        Real z;
        
        /// X component of the normal
        Real nX;
        
        /// Y component of the normal
        Real nY;
        
        /// Z component of the normal
        Real nZ;

        /** Convinience constructor.
        @param v
            The vertex position.
        @param n
            The vertex normal.
        */
        Vertex(const Vector3 &v, const Vector3 &n) :
            x(v.x), y(v.y), z(v.z),
            nX(n.x), nY(n.y), nZ(n.z)
        {
        }
    } Vertex;

    /** Leightweight triangle.
    */
    typedef struct _OgreVolumeExport Triangle
    {
        /// The first triangle corner.
        const Vertex mV1;
        
        /// The second triangle corner.
        const Vertex mV2;
        
        /// The third triangle corner.
        const Vertex mV3;
        
        /** Convinience constructor.
        @param v1
            The first vertex of the triangle.
        @param n1
            The normal of the first vertex.
        @param v2
            The second vertex of the triangle.
        @param n2
            The normal of the second vertex.
        @param v3
            The third vertex of the triangle.
        @param n3
            The normal of the third vertex.
        */
        Triangle(const Vector3 &v1, const Vector3 &n1, const Vector3 &v2, const Vector3 &n2, const Vector3 &v3, const Vector3 &n3) :
            mV1(v1, n1), mV2(v2, n2), mV3(v3, n3)
        {
        }
    } Triangle;

    /** == operator for two vertices.
    @param a
        The first vertex to test.
    @param b
        The second vertex to test.
    */
    bool _OgreVolumeExport operator==(Vertex const& a, Vertex const& b);
    
    /** A less operator. 
    @note
        This operator is needed so that Vertex can serve as the key in a map structrue 
    @param a
        The first vertex to test.
    @param b
        The second vertex to test.
    */
    bool _OgreVolumeExport operator<(const Vertex& a, const Vertex& b);

    /** To hold vertices.
    */
    typedef _OgreVolumeExport vector<Vertex>::type VecVertex;

    /** To hold indices.
    */
    typedef _OgreVolumeExport vector<size_t>::type VecIndices;

    /** Callback class when the user needs information about the triangles of
        chunks of a LOD level.
    */
    class _OgreVolumeExport MeshBuilderCallback
    {
    public:
        /** To be called with the callback function of a MeshBuilder.
        @param vertices
            Contains the vertices of the triangles.
        @param indices
            Contains the indices of the triangles.
        */
        virtual void trianglesReady(const VecVertex &vertices, const VecIndices &indices) = 0;
    };

    /** Class to build up a mesh with vertices and indices.
    */
    class _OgreVolumeExport MeshBuilder : public UtilityAlloc
    {
    protected:

        /// The buffer binding.
        static const unsigned short MAIN_BINDING;

        /// Map to get a vertex index.
        typedef map<Vertex, size_t>::type UMapVertexIndex;
        UMapVertexIndex mIndexMap;

         /// Holds the vertices of the mesh.
        VecVertex mVertices;

        /// Holds the indices of the mesh.
        VecIndices mIndices;

        /// Holds the bounding box.
        AxisAlignedBox mBox;

        /// Holds whether the initial bounding box has been set
        bool mBoxInit;
        
        /** Adds a vertex to the data structure, reusing the index if it is already known.
        @param v
            The vertex.
        */
        inline void addVertex(const Vertex &v)
        {
            size_t i = 0;
            if (mIndexMap.find(v) == mIndexMap.end())
            {
                i = mVertices.size();
                mIndexMap[v] = i;
                mVertices.push_back(v);

                // Update bounding box
                if (!mBoxInit)
                {
                    mBox.setExtents(v.x, v.y, v.z, v.x, v.y, v.z);
                    mBoxInit = true;
                }
                else
                {
                    if (v.x < mBox.getMinimum().x)
                    {
                        mBox.setMinimumX(v.x);
                    }
                    if (v.y < mBox.getMinimum().y)
                    {
                        mBox.setMinimumY(v.y);
                    }
                    if (v.z < mBox.getMinimum().z)
                    {
                        mBox.setMinimumZ(v.z);
                    }
                    if (v.x > mBox.getMaximum().x)
                    {
                        mBox.setMaximumX(v.x);
                    }
                    if (v.y > mBox.getMaximum().y)
                    {
                        mBox.setMaximumY(v.y);
                    }
                    if (v.z > mBox.getMaximum().z)
                    {
                        mBox.setMaximumZ(v.z);
                    }
                }
            }
            else
            {
                i = mIndexMap[v];
            }
            mIndices.push_back(i);
        }

    public:
        
        /** Adds a cube to a manual object rendering lines. Corner numeration:
             4 5
            7 6
             0 1
            3 2
        @param manual
            The manual for the cube lines.
        @param c0
            The corner 0.
        @param c1
            The corner 1.
        @param c2
            The corner 2.
        @param c3
            The corner 3.
        @param c4
            The corner 4.
        @param c5
            The corner 5.
        @param c6
            The corner 6.
        @param c7
            The corner 7.
        @param baseIndex
            The next free index of this manual object.
            Is incremented by 8 in this function.
        */
        static inline void addCubeToManualObject(
            ManualObject *manual,
            const Vector3 &c0,
            const Vector3 &c1,
            const Vector3 &c2,
            const Vector3 &c3,
            const Vector3 &c4,
            const Vector3 &c5,
            const Vector3 &c6,
            const Vector3 &c7,
            size_t &baseIndex
            )
        {
            manual->position(c0);
            manual->position(c1);
            manual->position(c2);
            manual->position(c3);
            manual->position(c4);
            manual->position(c5);
            manual->position(c6);
            manual->position(c7);

            manual->index(baseIndex + 0); manual->index(baseIndex + 1);
            manual->index(baseIndex + 1); manual->index(baseIndex + 2);
            manual->index(baseIndex + 2); manual->index(baseIndex + 3);
            manual->index(baseIndex + 3); manual->index(baseIndex + 0);
        
            manual->index(baseIndex + 4); manual->index(baseIndex + 5);
            manual->index(baseIndex + 5); manual->index(baseIndex + 6);
            manual->index(baseIndex + 6); manual->index(baseIndex + 7);
            manual->index(baseIndex + 7); manual->index(baseIndex + 4);
        
            manual->index(baseIndex + 0); manual->index(baseIndex + 4);
            manual->index(baseIndex + 1); manual->index(baseIndex + 5);
            manual->index(baseIndex + 2); manual->index(baseIndex + 6);
            manual->index(baseIndex + 3); manual->index(baseIndex + 7);
            baseIndex += 8;
        }

        /** Constructor.
        */
        MeshBuilder(void);
        
        /** Adds a triangle to the mesh with reusing already existent vertices via their index.
        @param t
            The triangle to add.
        */
        inline void addTriangle(Triangle t)
        {
            addVertex(t.mV1);
            addVertex(t.mV2);
            addVertex(t.mV3);
        }

        /** Generates the vertex- and indexbuffer of this mesh on the given
            RenderOperation.
        @param operation
            The RenderOperation for the buffers.
        @return
            The amount of generated triangles.
        */
        size_t generateBuffers(RenderOperation &operation);

        /** Generates an entity via a ManualObject.
        @param sceneManager
            The creating sceneManager.
        @param name
            The name for the entity.
        @param material
            The material to use.
        @return
            The created entity.
        */
        Entity* generateWithManualObject(SceneManager *sceneManager, const String &name, const String &material);

        /** Gets the bounding box of the mesh.
        @return
            The bounding box.
        */
        AxisAlignedBox getBoundingBox(void);

        /** Executes a MeshBuilderCallback on this instance.
         @param callback
            The callback to execute.
        */
        void executeCallback(MeshBuilderCallback *callback) const;

    };
}
}

#endif