#include "terrainhandler.h"
/*
    This class requires the terrain.* shader in the
    bin/Shaders folder to work properly
*/

TerrainHandler::TerrainHandler()
{
    GPUDataSet=false;
    GPUTexSet=false;
    GPUShdrSet=false;
}

//*********************************************
//       Sets up the Shader and Textures
//*********************************************
void TerrainHandler::SetTextures()
{
    if (!GPUTexSet)
    {
        Console::cPrint("Setting Textures on GPU...");
        //std::cout << "Setting Textures on GPU...\n";

        // Setup Textures
        texture[0].Setup(tools::appendStrings("landscape/",TextureFiles[0]),"textures.lowlandMap");
        texture[1].Setup(tools::appendStrings("landscape/",TextureFiles[1]),"textures.mediumlandMap");
        texture[2].Setup(tools::appendStrings("landscape/",TextureFiles[2]),"textures.highlandMap");
        texture[3].Setup(tools::appendStrings("landscape/",TextureFiles[3]),"textures.cliffMap");

        // Load Textures to CPU and GPU
        for (auto&& tex : texture)
        {
            tex.LoadTextureDataToCPU();
            tex.LoadTextureDataToGPU();
        }

        GPUTexSet=true;
    }
    else
    {
        Console::cPrint("Textures already set on the GPU!");
        //std::cout << "Textures already set on the GPU!\n";
    }
};

//*********************************************
//       Sets up the Shader and Textures
//*********************************************
void TerrainHandler::SetShader()
{
    if (!GPUShdrSet)
    {
        //std::cout << "Setting Shader on GPU...\n";
        Console::cPrint("Setting Shader on GPU...");
        shader.ShaderSet(ShaderFiles);
        GPUShdrSet=true;
    }
    else
    {
        Console::cPrint("Shader already set on the GPU!");
        //std::cout << "Shader already set on the GPU!\n";
    }
};

//*********************************************
//       Unset the Textures
//*********************************************
void TerrainHandler::UnsetTextures()
{
    if (GPUTexSet)
    {
        Console::cPrint("Clearing Textures on GPU...");
        //std::cout << "Clearing Textures on GPU...\n";
        for (auto&& tex : texture)
            tex.TextureCleanup();
        GPUTexSet=false;
    }
    else
    {
        Console::cPrint("Textures not set on the GPU.");
        //std::cout << "Textures not set on the GPU.\n";
    }
};

//*********************************************
//       Unset the Shader
//*********************************************
void TerrainHandler::UnsetShader()
{
    if (GPUShdrSet)
    {
        Console::cPrint("Clearing Shader on GPU...");
        //std::cout << "Clearing Shader on GPU...\n";
        shader.Cleanup();
        GPUShdrSet=false;
    }
    else
    {
        Console::cPrint("Shader not set on GPU.");
        //std::cout << "Shader not set on GPU.\n";
    }
};

//*********************************************
//              Set Materials on GPU
//*********************************************
/*
Set Materials on GPU
*/
void TerrainHandler::setMaterialUniform ()
{
    GLuint Prog = shader.Program;
    GLint AmbientLoc = glGetUniformLocation(Prog, "light.ambient");
    GLint DiffuseLoc = glGetUniformLocation(Prog, "light.diffuse");
    GLint SpecularLoc = glGetUniformLocation(Prog, "light.specular");
    GLint ShineLoc = glGetUniformLocation(Prog, "light.shininess");

    //cout << "Ambient [" << materials.Ka.x << "," << materials.Ka.y << "," << materials.Ka.z << "]\n";

    glUniform3f(AmbientLoc, materials.Ka.x, materials.Ka.y, materials.Ka.z);
    glUniform3f(DiffuseLoc, materials.Kd.x, materials.Kd.y, materials.Kd.z);
    glUniform3f(SpecularLoc, materials.Ks.x, materials.Ks.y, materials.Ks.z);
    glUniform1f(ShineLoc, materials.shine);
};

//*********************************************
//      Set Relative Height Data on GPU
//*********************************************
/*
Set Relative Height Data on GPU.
*/
void TerrainHandler::SetRelativeHeightUniform()
{
    GLuint Prog = shader.Program;
    GLint RelHeightLoc = glGetUniformLocation(Prog, "textures.relativeHeight");
    glUniform4f(RelHeightLoc,relativeHeight.x,relativeHeight.y,relativeHeight.z,relativeHeight.w);
};

//*********************************************
//            Setup Mesh on GPU
//*********************************************
/*
Average the heights, this helps smooth the terrain
after generation.
*/
void TerrainHandler::setupMeshes()
{
    Console::cPrint("Setting Up Terrain Mesh");
    //std::cout << "Setting Up Terrain Mesh" << std::endl;
    // Create buffers/arrays
    for (int i=0; i<int(meshVerts.size()); ++i)
    {
        ogltools::BufferHandler tmpBuff;
        tmpBuff.GenBuffers(meshVerts[i],idxs[i]);
        buffers.push_back(tmpBuff);
    }
};

//*********************************************
//         Move Terrain Data to GPU
//*********************************************
void TerrainHandler::SetTerrainOnGPU()
{
    if (GPUDataSet)
    {
        while (!buffers.empty())
        {
            buffers.back().ClearBuffers();
            buffers.pop_back();
        }

        setupMeshes();
    }
    else
    {
        setupMeshes();
    }

    GPUDataSet=true;
};

//*********************************************
//              Setup Tex Files
//*********************************************
/*
Setup Tex Files
*/
void TerrainHandler::SetTexFiles(std::string f1,std::string f2,std::string f3,std::string f4)
{
    TextureFiles[0]=f1;
    TextureFiles[1]=f2;
    TextureFiles[2]=f3;
    TextureFiles[3]=f4;
};

//*********************************************
//              Setup Materials
//*********************************************
/*
    Ka = Ambient light vector (glm::vec3)
    Kd = Diffuse light vector (glm::vec3)
    Ks = Specular light vector (glm::vec3)
    shininess = Specular shine factor (float)
*/
void TerrainHandler::SetupMaterials(glm::vec3 Ka,glm::vec3 Kd,glm::vec3 Ks,float shininess)
{
    materials.Ka = Ka;
    materials.Kd = Kd;
    materials.Ks = Ks;
    materials.shine = shininess;
};

//*********************************************
//     Setup Mesh Subdivision Information
//*********************************************
/*

*/
void TerrainHandler::SetupMeshSubInfo(int Nsub,int Elen,float Vdist)
{
    this->Nsub = Nsub;
    this->Elen = Elen;
    this->Vdist = Vdist;

    //Compute some required variables
    this->Elenf = 1.0/((Elen-1)*double(Vdist)); // Edge length of a mesh
    this->MaxL = Nsub/(float)2;
};

//*********************************************
//             Draw the Terrain
//*********************************************
void TerrainHandler::DrawCall()
{
    if (GPUDataSet)
    {
        Draw();
    }
};

//*********************************************
//         Draws the Mesh (Singular)
//*********************************************
/*
Draw the mesh to the color buffer.
*/
void TerrainHandler::Draw()
{
    // Use Shader
    shader.Use();

    // Bind Textures
    texture[0].useTexture(shader,0);
    texture[1].useTexture(shader,1);
    texture[2].useTexture(shader,2);
    texture[3].useTexture(shader,3);

    // Set Materials
    setMaterialUniform();

    // Set Relative Height
    SetRelativeHeightUniform();

    // Create buffers/arrays
    for (int i=0; i<(int)buffers.size(); ++i)
    {
        // Set Position
        GLuint modelLoc = glGetUniformLocation(shader.Program, "modelMat");

        glm::mat4 model2;
        model2=glm::translate(model2,glm::vec3(0.0f,0.0f,0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));

        // Draw mesh
        buffers[i].DrawVerts();
    }
};

//*********************************************
//              Cleanup Class
//*********************************************
void TerrainHandler::Cleanup()
{
    if (GPUDataSet)
    {
        while (!buffers.empty())
        {
            buffers.back().ClearBuffers();
            buffers.pop_back();
        }

        GPUDataSet=false;
    }

    UnsetTextures();
    UnsetShader();
};

//**************************
//  Get GPU Requirements
//**************************
long int TerrainHandler::GetGPUMemoryReqs()
{
    int rtnval=0;

    if (GPUDataSet)
    {
        rtnval += sizeof(Vertex) * this->meshVerts.back().size() * this->buffers.size();
        rtnval += sizeof(GLuint) * this->idxs.back().size() * this->buffers.size();
        rtnval /= (1024*1024);
    }

    for (int i=0;i<4;++i)
        rtnval += texture[i].MemSize();

    return rtnval;
};

//**************************
//     Number of Verts
//**************************
int TerrainHandler::GetNumberVerts()
{
    int rtnval=0;

    if (GPUDataSet)
    {
        rtnval = this->meshVerts.back().size() * this->buffers.size();
    }

    return rtnval;
};

//******************************************
//  Get the Mesh ID at a given x,z Position
//******************************************
int TerrainHandler::GetMeshVertIDatPos(double x, double z,glm::ivec3 &vt)
{
    //optTimer.start_point();
    double xtrav = x*Elenf; //A normalized mesh coordinate which numbers out the meshes
    double ztrav = z*Elenf;

    if ( fabs(xtrav)>MaxL || fabs(ztrav)>MaxL )
    {
        return -1;
    } else {
        xtrav += MaxL; // Shift coords back to original (+ only) mesh coords
        ztrav += MaxL;

        int Nx = floor(xtrav);
        int Nz = floor(ztrav);

        int MID = Nx+Nsub*Nz; // Mesh ID

        double xRem = xtrav - Nx;
        double zRem = ztrav - Nz;

        int Em1 = (Elen-1);

        int row = int(floor(Em1*xRem));
        int col = int(floor(Em1*zRem));

        int ielem = row+col*Elen;

        xRem = Em1*xRem - double(row);
        zRem = Em1*zRem - double(col);

        // Determine which triangle we are searching for
        if (xRem+zRem > 1.0)
        {
            vt.x = ielem+1;
            vt.y = ielem+Elen;
            vt.z = ielem+Elen+1;
            //Console::cPrint(tools::appendStrings(" SECOND: ",xRem + zRem," Tri: ",verts[0],",",verts[1],",",verts[2]));
        } else {
            vt.x = ielem;
            vt.y = ielem+1;
            vt.z = ielem+Elen;
            //Console::cPrint(tools::appendStrings(" FIRST: ",xRem + zRem," Tri: ",verts[0],",",verts[1],",",verts[2]));
        }

        //Console::cPrint(tools::appendStrings(" MeshID: ",MID," Tri: ",verts[0],",",verts[1],",",verts[2]));

        //optTimer.end_point();
        //Console::cPrint(optTimer.get_generic_print_string("modifyElevation "));

        return MID;
    }
};

//**************************
//    Export the Terrain
//**************************
void TerrainHandler::ExportTerrain(std::string filename)
{
    std::stringstream ss;
    ss << "../Data/Terrain/" << filename << ".ter";

    std::ofstream savefile;
    savefile.open(ss.str().c_str());

    savefile << "L1Tex=" << TextureFiles[0] << std::endl;
    savefile << "L2Tex=" << TextureFiles[1] << std::endl;
    savefile << "L3Tex=" << TextureFiles[2] << std::endl;
    savefile << "L4Tex=" << TextureFiles[3] << std::endl;

    savefile << "Shdr=" << ShaderFiles << std::endl;

    savefile << "mKa=" << materials.Ka.x << "," << materials.Ka.y << "," << materials.Ka.z << std::endl;
    savefile << "mKd=" << materials.Kd.x << "," << materials.Kd.y << "," << materials.Kd.z << std::endl;
    savefile << "mKs=" << materials.Ks.x << "," << materials.Ks.y << "," << materials.Ks.z << std::endl;
    savefile << "mSh=" << materials.shine << std::endl;

    savefile << "rHx=" << relativeHeight.x << std::endl;
    savefile << "rHy=" << relativeHeight.y << std::endl;
    savefile << "rHz=" << relativeHeight.z << std::endl;
    savefile << "rHw=" << relativeHeight.w << std::endl;

    // Save Indicies
    int Nmesh=meshVerts.size();
    int Nvert=meshVerts[0].size();
    int Nidxs=idxs[0].size();

    savefile << "Nmesh=" << Nmesh << std::endl;
    savefile << "Nidx=" << Nvert << std::endl;

    savefile << "$BEGIN_IDXS" << std::endl;
    for (int i=0;i<Nmesh;++i)
    {
        savefile << "$MESH_" << i << std::endl;
        for (int j=0;j<Nidxs;)
        {
            savefile << " " << idxs[i][j] << " " <<idxs[i][j+1] << " " << idxs[i][j+2] << std::endl;
            j+=3;
        }
    }
    savefile << "$END_IDXS" << std::endl;

    savefile << "Nvert=" << Nvert << std::endl;

    savefile << "$BEGIN_VERTS" << std::endl;
    for (int i=0;i<Nmesh;++i)
    {
        savefile << "$MESH_" << i << std::endl;
        for (int j=0;j<Nvert;++j)
        {
            savefile << " " << meshVerts[i][j].position.x << " " << meshVerts[i][j].position.y << " " << meshVerts[i][j].position.z << std::endl;
            savefile << " " << meshVerts[i][j].normal.x << " " << meshVerts[i][j].normal.y << " " << meshVerts[i][j].normal.z << std::endl;
            savefile << " " << meshVerts[i][j].texture.x << " " << meshVerts[i][j].texture.y << std::endl;
        }
    }
    savefile << "$END_VERTS" << std::endl;

    savefile.close();
};
