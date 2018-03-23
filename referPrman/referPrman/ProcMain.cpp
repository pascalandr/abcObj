#include "traverse.h"
#include <iostream>
#include <fstream>
using namespace std;
#include <streambuf>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/AbcGeom/Foundation.h>
using namespace Alembic::AbcGeom;

//walk,visit�����ǲο�abcImPort�ķ���

void walk(Alembic::Abc::IArchive & iRoot)
{
	IObject mParent=iRoot.getTop();


	if(!iRoot.valid())
	{
		cout<<"�ĵ���Ч��û�и��ڵ㣡"<<endl;
	}
	//Ԥ���ػ����νṹ
	AlembicObjectPtr topObject=
		previsit(AlembicObjectPtr(new AlembicObject(iRoot.getTop())));

	if(!topObject)
		cout<<"topObjectΪ��"<<endl;

	size_t numChildren=topObject->getNumChildren();
	//cout<<"numChildren�������ǣ�"<<numChildren<<endl;
	if(numChildren==0)
	{
		cout<<"numChildrenΪ��"<<endl;
	}

	for(size_t i=0;i<numChildren;i++)
	{
		visit(topObject->getChild(i));
	}
}


//�ֶ���д����
void subdivide()
{
	std::string workString;
	std::string filePath;
	try
	{
	Alembic::AbcCoreFactory::IFactory factory;
	IArchive archive=factory.getArchive("C:/Users/KCM/Desktop/transobj/test/cube/cube.abc");
	walk(archive);

	}//try�����ĵط�
    catch ( const std::exception & e )
    {
       std::cerr << "AlembicRiProcedural: cannot read arguments file: ";
       std::cerr << filePath << std::endl;
	}
}



void main()
{
	subdivide();
	system("pause");
	return ;
}