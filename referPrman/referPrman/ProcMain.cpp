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

//walk,visit函数是参考abcImPort的方法

void walk(Alembic::Abc::IArchive & iRoot)
{
	IObject mParent=iRoot.getTop();


	if(!iRoot.valid())
	{
		cout<<"文档无效，没有根节点！"<<endl;
	}
	//预加载缓存层次结构
	AlembicObjectPtr topObject=
		previsit(AlembicObjectPtr(new AlembicObject(iRoot.getTop())));

	if(!topObject)
		cout<<"topObject为空"<<endl;

	size_t numChildren=topObject->getNumChildren();
	//cout<<"numChildren的数量是："<<numChildren<<endl;
	if(numChildren==0)
	{
		cout<<"numChildren为空"<<endl;
	}

	for(size_t i=0;i<numChildren;i++)
	{
		visit(topObject->getChild(i));
	}
}


//手动编写参数
void subdivide()
{
	std::string workString;
	std::string filePath;
	try
	{
	Alembic::AbcCoreFactory::IFactory factory;
	IArchive archive=factory.getArchive("C:/Users/KCM/Desktop/transobj/test/cube/cube.abc");
	walk(archive);

	}//try结束的地方
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