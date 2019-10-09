/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004-2016                                           \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

/****************************************************************************
History

$Log: not supported by cvs2svn $
Revision 1.11  2006/03/29 09:27:07  cignoni
Added managemnt of non critical errors

Revision 1.10  2006/03/29 08:16:31  corsini
Minor change in LoadMask

Revision 1.9  2006/03/27 07:17:49  cignoni
Added generic LoadMask

Revision 1.8  2006/03/07 13:19:29  cignoni
First Release with OBJ import support

Revision 1.7  2006/02/28 14:50:00  corsini
Fix comments

Revision 1.6  2006/02/10 16:14:53  corsini
Fix typo

Revision 1.5  2006/02/10 08:14:32  cignoni
Refactored import. No more duplicated code

Revision 1.4  2006/02/09 16:04:45  corsini
Expose load mask

Revision 1.3  2006/01/11 10:37:45  cignoni
Added use of Callback

Revision 1.2  2005/01/26 22:43:19  cignoni
Add std:: to stl containers

Revision 1.1  2004/11/29 08:12:10  cignoni
Initial Update


****************************************************************************/

#ifndef __VCGLIB_IMPORT_BASE
#define __VCGLIB_IMPORT_BASE

#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/import_ply.h>
#include <wrap/io_trimesh/import_stl.h>
#include <wrap/io_trimesh/import_off.h>
#include <wrap/io_trimesh/import_vmi.h>
#include "..\include\import_3ds.h"

#include <locale>

namespace vcg {
	namespace tri {
		namespace io {

			/**
			This class encapsulate a filter for automatically importing meshes by guessing
			the right filter according to the extension
			*/

			template <class OpenMeshType>
			class Importer
			{
			private:
				enum KnownTypes {
					KT_UNKNOWN, KT_PLY, KT_STL, KT_OFF, KT_OBJ, KT_VMI, KT_3DS
				};
				static int &LastType()
				{
					static int lastType = KT_UNKNOWN;
					return lastType;
				}

			public:
				enum ImporterError {
					E_NOERROR = 0 // No error =0 is the standard for ALL the imported files.
				};
				// simple aux function that returns true if a given file has a given extesnion
				static bool FileExtension(std::string filename, std::string extension)
				{
					std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
					std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
					std::string end = filename.substr(filename.length() - extension.length(), extension.length());
					return end == extension;
				}
				// Open Mesh, returns 0 on success.
				static int Open(OpenMeshType &m, const char *filename, CallBackPos *cb = 0)
				{
					int dummymask = 0;
					return Open(m, filename, dummymask, cb);
				}

				/// Open Mesh and fills the load mask (the load mask must be initialized first); returns 0 on success.
				static int Open(OpenMeshType &m, const char *filename, int &loadmask, CallBackPos *cb = 0)
				{
					int err;
					if (FileExtension(filename, "ply"))
					{
						err = ImporterPLY<OpenMeshType>::Open(m, filename, loadmask, cb);
						LastType() = KT_PLY;
					}
					else if (FileExtension(filename, "stl"))
					{
						err = ImporterSTL<OpenMeshType>::Open(m, filename, loadmask, cb);
						LastType() = KT_STL;
					}
					else if (FileExtension(filename, "off"))
					{
						err = ImporterOFF<OpenMeshType>::Open(m, filename, loadmask, cb);
						LastType() = KT_OFF;
					}
					else if (FileExtension(filename, "obj"))
					{
						err = ImporterOBJ<OpenMeshType>::Open(m, filename, loadmask, cb);
						LastType() = KT_OBJ;
					}
					else if (FileExtension(filename, "vmi"))
					{
						err = ImporterVMI<OpenMeshType>::Open(m, filename, loadmask, cb);
						LastType() = KT_VMI;
					}
					else if (FileExtension(filename, "3ds"))
					{
						vcg::tri::io::_3dsInfo info;
						info.cb = cb;
						Lib3dsFile *file = NULL;

						file = lib3ds_file_load(filename);
						if (!file)
						{
							err = vcg::tri::io::Importer3DS<OpenMeshType>::E_CANTOPEN;
							return err;
						}

						// No nodes?  Fabricate nodes to display all the meshes.
						if (!file->nodes && file->meshes)
						{
							Lib3dsMesh *mesh;
							Lib3dsNode *node;

							for (mesh = file->meshes; mesh != NULL; mesh = mesh->next)
							{
								node = lib3ds_node_new_object();
								strcpy(node->name, mesh->name);
								node->parent_id = LIB3DS_NO_PARENT;
								lib3ds_file_insert_node(file, node);
							}
						}

						if (!file->nodes)
						{
							err = vcg::tri::io::Importer3DS<OpenMeshType>::E_NO_VERTEX;
							return err;
						}

						lib3ds_file_eval(file, 0);

						bool normalsUpdated = false;
						vcg::tri::io::Importer3DS<OpenMeshType>::LoadMask(file, 0, info);
						//enable mask
						{							
							if ((info.mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD) != 0)
								m.face.EnableWedgeTexCoord();
							if ((info.mask & vcg::tri::io::Mask::IOM_FACENORMAL) != 0)
								m.face.EnableNormal();
							if ((info.mask & vcg::tri::io::Mask::IOM_WEDGNORMAL) != 0)
								m.face.EnableWedgeNormal();
							if ((info.mask & vcg::tri::io::Mask::IOM_FACECOLOR) != 0)
								m.face.EnableColor(); 
							if ((info.mask & vcg::tri::io::Mask::IOM_VERTCOLOR) != 0)
								m.vert.EnableColor();
							if ((info.mask & vcg::tri::io::Mask::IOM_VERTTEXCOORD) != 0)
								m.vert.EnableTexCoord();
						}
						err = vcg::tri::io::Importer3DS<OpenMeshType>::Load(m, file, 0, info);
						if (err != vcg::tri::io::Importer3DS<OpenMeshType>::E_NOERROR)
						{
							lib3ds_file_free(file);
							return err;
						}

						if (info.mask & vcg::tri::io::Mask::IOM_WEDGNORMAL)
							normalsUpdated = true;

						loadmask = info.mask;

						std::string missingTextureFilesMsg = "The following texture files were not found:\n";
						bool someTextureNotFound = false;
						for (unsigned textureIdx = 0; textureIdx < m.textures.size(); ++textureIdx)
						{
							FILE* pFile = fopen(m.textures[textureIdx].c_str(), "r");
							if (pFile == NULL)
							{
								missingTextureFilesMsg.append("\n");
								missingTextureFilesMsg.append(m.textures[textureIdx].c_str());
								someTextureNotFound = true;
							}
							fclose(pFile);
						}

						if (someTextureNotFound) {
							std::cout << "warning: \n" << missingTextureFilesMsg << std::endl;
						}

						vcg::tri::UpdateBounding<OpenMeshType>::Box(m);					// updates bounding box
						if (!normalsUpdated)
							vcg::tri::UpdateNormal<OpenMeshType>::PerVertex(m);		// updates normals

						if (cb != NULL)	(*cb)(99, "Done");

						// freeing memory
						lib3ds_file_free(file);
						LastType() = KT_3DS;
					}
					else {
						err = 1;
						LastType() = KT_UNKNOWN;
					}

					return err;
				}

				static bool ErrorCritical(int error)
				{
					switch (LastType())
					{
					case KT_PLY: return (error > 0); break;
					case KT_STL: return (error > 0); break;
					case KT_OFF: return (error > 0); break;
					case KT_OBJ: return ImporterOBJ<OpenMeshType>::ErrorCritical(error); break;
					case KT_3DS: return (error > 0); break;
					}

					return true;
				}

				static const char *ErrorMsg(int error)
				{
					switch (LastType())
					{
					case KT_PLY: return ImporterPLY<OpenMeshType>::ErrorMsg(error); break;
					case KT_STL: return ImporterSTL<OpenMeshType>::ErrorMsg(error); break;
					case KT_OFF: return ImporterOFF<OpenMeshType>::ErrorMsg(error); break;
					case KT_OBJ: return ImporterOBJ<OpenMeshType>::ErrorMsg(error); break;
					case KT_VMI: return ImporterVMI<OpenMeshType>::ErrorMsg(error); break;
					case KT_3DS: return Importer3DS<OpenMeshType>::ErrorMsg(error); break;

					}
					return "Unknown type";
				}

				static bool LoadMask(const char * filename, int &mask)
				{
					bool err;

					if (FileExtension(filename, "ply"))
					{
						err = ImporterPLY<OpenMeshType>::LoadMask(filename, mask);
						LastType() = KT_PLY;
					}
					else if (FileExtension(filename, "stl"))
					{
						mask = Mask::IOM_VERTCOORD | Mask::IOM_FACEINDEX;
						err = true;
						LastType() = KT_STL;
					}
					else if (FileExtension(filename, "off"))
					{
						mask = Mask::IOM_VERTCOORD | Mask::IOM_FACEINDEX;
						err = ImporterOFF<OpenMeshType>::LoadMask(filename, mask);
						LastType() = KT_OFF;
					}
					else if (FileExtension(filename, "obj"))
					{
						err = ImporterOBJ<OpenMeshType>::LoadMask(filename, mask);
						LastType() = KT_OBJ;
					}
					else
					{
						err = false;
						LastType() = KT_UNKNOWN;
					}

					return err;
				}
			}; // end class
		} // end Namespace tri
	} // end Namespace io
} // end Namespace vcg

#endif
