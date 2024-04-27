#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace PolygonalLibrary {

bool ImportMesh(const string& filepath,
                PolygonalMesh& mesh)
{

    if(!ImportCell0Ds(filepath + "/Cell0Ds.csv",
                       mesh))
        return false;
    else
    {
        // controllo che i markers siano stati salvati correttamente
        cout << "Cell0D marker:" << endl;
        for(auto it = mesh.Cell0DMarkers.begin(); it != mesh.Cell0DMarkers.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;

            cout << endl;
        }
    }

    if(!ImportCell1Ds(filepath + "/Cell1Ds.csv",
                       mesh))
        return false;
    else
    {
        // controllo che i markers siano stati salvati correttamente
        cout << "Cell1D marker:" << endl;
        for(auto it = mesh.Cell1DMarkers.begin(); it != mesh.Cell1DMarkers.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;

            cout << endl;
        }
    }

    if(!ImportCell2Ds(filepath + "/Cell2Ds.csv",
                       mesh))
        return false;
    else
    {
        // controllo che i markers siano stati salvati correttamente
        cout << "Cell2D marker:" << endl;
        for(auto it = mesh.Cell2DMarkers.begin(); it != mesh.Cell2DMarkers.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;

            cout << endl;
        }

        for(unsigned int c = 0; c < mesh.NumberCell2D; c++)
        {
            vector<unsigned int> edges = mesh.Cell2DEdges[c];
            unsigned int ne = edges.size();

            vector<unsigned int> vertices = mesh.Cell2DVertices[c];
            unsigned int nv = vertices.size();

            double area = 0;

            // calcolo l'area
            for(unsigned int v = 0; v < nv; v++)
            {
                const double xCoord = mesh.Cell0DCoordinates[vertices[v]][0];
                const double yCoord = mesh.Cell0DCoordinates[vertices[v]][1];

                double xNextCoord;
                double yNextCoord;

                if(v == nv - 1)
                {
                    xNextCoord = mesh.Cell0DCoordinates[vertices[0]][0];
                    yNextCoord = mesh.Cell0DCoordinates[vertices[0]][1];
                }
                else
                {
                    xNextCoord = mesh.Cell0DCoordinates[vertices[v + 1]][0];
                    yNextCoord = mesh.Cell0DCoordinates[vertices[v + 1]][1];
                }

                area += xCoord * yNextCoord - xNextCoord * yCoord;
            }

            area = abs(area) / 2;

            // controllo che l'area non sia nulla
            if(area < 1.1e-16)
            {
                cerr << "Null area" << endl;
                return 5;
            }

            for(unsigned int e = 0; e < ne; e++)
            {
                const unsigned int origin = mesh.Cell1DVertices[edges[e]][0];
                const unsigned int end = mesh.Cell1DVertices[edges[e]][1];

                // controllo che i lati non abbiano lunghezza nulla
                if(origin == end)
                {
                    cerr << "Null edge" << endl;
                    return 2;
                }

                // controllo che i vertici corrispondano agli estremi dei lati
                auto findOrigin = find(mesh.Cell2DVertices[c].begin(), mesh.Cell2DVertices[c].end(), origin);
                if(findOrigin == mesh.Cell2DVertices[c].end())
                {
                    cerr << "Wrong mesh" << endl;
                    return 3;
                }

                auto findEnd = find(mesh.Cell2DVertices[c].begin(), mesh.Cell2DVertices[c].end(), end);
                if(findEnd == mesh.Cell2DVertices[c].end())
                {
                    cerr << "Wrong mesh" << endl;
                    return 4;
                }
            }
        }
    }

    return true;
}

bool ImportCell0Ds(const string &filename,
                   PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }

    file.close();

    listLines.pop_front();

    mesh.NumberCell0D = listLines.size();

    if (mesh.NumberCell0D == 0)
    {
        cerr << "There is no cell 0D" << endl;
        return false;
    }

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2d coord;

        converter >> id >> marker >> coord(0) >> coord(1);

        mesh.Cell0DId.push_back(id);
        mesh.Cell0DCoordinates.push_back(coord);

        if( marker != 0)
        {
            if (mesh.Cell0DMarkers.find(marker) == mesh.Cell0DMarkers.end())
                mesh.Cell0DMarkers.insert({marker, {id}});
            else
                mesh.Cell0DMarkers[marker].push_back(id);
        }
    }

    file.close();
    return true;
}


bool ImportCell1Ds(const string &filename,
                   PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }

    listLines.pop_front();

    mesh.NumberCell1D = listLines.size();

    if (mesh.NumberCell1D == 0)
    {
        cerr << "There is no cell 1D" << endl;
        return false;
    }

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2i vertices;

        converter >> id >> marker >> vertices(0) >> vertices(1);

        mesh.Cell1DId.push_back(id);
        mesh.Cell1DVertices.push_back(vertices);

        if(marker != 0)
        {
            if (mesh.Cell1DMarkers.find(marker) == mesh.Cell1DMarkers.end())
                mesh.Cell1DMarkers.insert({marker, {id}});
            else
                mesh.Cell1DMarkers[marker].push_back(id);
        }
    }

    file.close();
    return true;
}


bool ImportCell2Ds(const string &filename,
                   PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }

    listLines.pop_front();

    mesh.NumberCell2D = listLines.size();

    if (mesh.NumberCell2D == 0)
    {
        cerr << "There is no cell 2D" << endl;
        return false;
    }

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        unsigned int numvertices;
        unsigned int numedges;

        converter >> id >> marker >> numvertices;

        vector<unsigned int> vertices(numvertices) ;

        for(unsigned int i = 0; i < numvertices; i++)
            converter >> vertices[i];

        converter >> numedges;
        vector<unsigned int> edges(numedges) ;

        for(unsigned int i = 0; i < numedges; i++)
            converter >> edges[i];

        mesh.Cell2DId.push_back(id);
        mesh.Cell2DVertices.push_back(vertices);
        mesh.Cell2DEdges.push_back(edges);

        if( marker != 0)
        {
            if (mesh.Cell2DMarkers.find(marker) == mesh.Cell2DMarkers.end())
                mesh.Cell2DMarkers.insert({marker, {id}});
            else
                mesh.Cell2DMarkers[marker].push_back(id);
        }
    }

    file.close();
    return true;
}
}
