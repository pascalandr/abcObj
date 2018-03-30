#include "animatePorcess.h"
#include "global.h"
#include <fstream>
#include <iomanip>
#include <Alembic/AbcGeom/All.h>

using namespace std;

//参考prman
void GetRelevantSampleTimes(TimeSamplingPtr timeSampling,
							size_t numSamples, SampleTimeSet &output)
{
	if(numSamples<2)
	{
		output.insert(0.0);
		return;
	}
	chrono_t frameTime=frame/24.0;//相当于这一帧所在的时间
	chrono_t shutterOpenTime=(frame+0.0)/24.0;
	chrono_t shutterCloseTime=(frame+0.5)/24.0;
	std::pair<index_t,chrono_t> shutterOpenFloor=timeSampling->getFloorIndex(shutterOpenTime,numSamples);//取下整
	std::pair<index_t,chrono_t> shutterCloseCeil=timeSampling->getCeilIndex(shutterCloseTime,numSamples);//取上整

	//static const chrono_t epsilon=1.0/10000.0;
	//检查第二个样本是否是我们想要的，坡点幅度至少确保我们有两个样本可以使用。
	//if(shutterOpenFloor.first<shutterCloseCeil.first)
	//{
	//	//如果我们的样本少于开放时间，请看下一个索引时间
	//	if(shutterOpenFloor.second<shutterOpenTime)
	//	{
	//		chrono_t nextSampleTime=
	//			timeSampling->getSampleTime(shutterOpenFloor.first+1);
	//		if(fabs(nextSampleTime-shutterOpenTime)<epsilon)
	//		{
	//			shutterOpenFloor.first+=1;
	//			shutterOpenFloor.second=nextSampleTime;
	//		}
	//	}
	//}

	for(index_t i=shutterOpenFloor.first;i<shutterCloseCeil.first;++i)
	{
		output.insert(timeSampling->getSampleTime(i));//获得任何样本的时间
	}


	if(output.size()==0)
	{
		output.insert(frameTime);
		return;
	}

	//chrono_t lastSample= *(output.rbegin());
	//if((fabs(lastSample-shutterCloseTime)>epsilon )
	//	&& lastSample<shutterCloseTime)
	//{
	//	output.insert(shutterCloseCeil.second);
	//}
}

double getWeightAndIndex(double iFrame,
    Alembic::AbcCoreAbstract::TimeSamplingPtr iTime, size_t numSamps,
    Alembic::AbcCoreAbstract::index_t & oIndex,
    Alembic::AbcCoreAbstract::index_t & oCeilIndex)
{
    if (numSamps == 0)
        numSamps = 1;

	//找到时间小于或等于给定时间的最大有效索引，使用零个样本无效
	//如果最小采样时间大于iFrame，则返回0
    std::pair<Alembic::AbcCoreAbstract::index_t, double> floorIndex =
        iTime->getFloorIndex(iFrame, numSamps);

    oIndex = floorIndex.first;
    oCeilIndex = oIndex;

    if (fabs(iFrame - floorIndex.second) < 0.0001)
        return 0.0;

    std::pair<Alembic::AbcCoreAbstract::index_t, double> ceilIndex =
        iTime->getCeilIndex(iFrame, numSamps);

    if (oIndex == ceilIndex.first)
        return 0.0;

    oCeilIndex = ceilIndex.first;

    double alpha = (iFrame - floorIndex.second) /
        (ceilIndex.second - floorIndex.second);

    // we so closely match the ceiling so we'll just use it
    if (fabs(1.0 - alpha) < 0.0001)
    {
        oIndex = oCeilIndex;
        return 0.0;
    }

    return alpha;
}


void setUVs(double iFrame,Alembic::AbcGeom::IV2fGeomParam iUVs,string filename)
{
	ofstream outfile;
	outfile.open(filename,ios::app);
	if(!iUVs.valid())
	{
		cout<<"uv无效"<<endl;
		return;
	}
	Alembic::AbcCoreAbstract::index_t index,ceilIndex;
	getWeightAndIndex(iFrame,iUVs.getTimeSampling(),iUVs.getNumSamples(),index,ceilIndex);
	Alembic::AbcGeom::IV2fGeomParam::Sample samp;
    iUVs.getIndexed(samp, Alembic::Abc::ISampleSelector(index));

    Alembic::AbcGeom::V2fArraySamplePtr uvPtr = samp.getVals();
    Alembic::Abc::UInt32ArraySamplePtr indexPtr = samp.getIndices();
    unsigned int numUVs = (unsigned int)uvPtr->size();
    for (unsigned int i = 0; i < numUVs; ++i)
     {
            outfile<<"vt "<<fixed<<(*uvPtr)[i].x<<" ";
            outfile<<"vt "<<fixed<<(*uvPtr)[i].y<<endl;
     }
}

void setPolyNormals(double iFrame,Alembic::AbcGeom::IN3fGeomParam iNormals,string filename)
{
	ofstream outfile;
	outfile.open(filename,ios::app);
	if(!iNormals)
	{
		cout<<"normal无效！"<<endl;
		return ;
	}

	if (iNormals.getScope() != Alembic::AbcGeom::kVertexScope &&
            iNormals.getScope() != Alembic::AbcGeom::kVaryingScope &&
            iNormals.getScope() != Alembic::AbcGeom::kFacevaryingScope)
        {
           cout<<"normal vector has an unsupported scope, skipping normals!"<<endl;
		   return ;
        }


	Alembic::AbcCoreAbstract::index_t index,ceilIndex;
	 double alpha = getWeightAndIndex(iFrame,
            iNormals.getTimeSampling(), iNormals.getNumSamples(),
            index, ceilIndex);
	 Alembic::AbcGeom::IN3fGeomParam::Sample samp;
	 iNormals.getExpanded(samp,Alembic::Abc::ISampleSelector(index));

	 Alembic::Abc::N3fArraySamplePtr sampVal = samp.getVals();

     size_t sampSize = sampVal->size();
	 cout<<"normalNum:"<<sampSize<<endl;

	 Alembic::Abc::N3fArraySamplePtr ceilVals;
	 if(alpha != 0 && index != ceilIndex)
	 {
		 Alembic::AbcGeom::IN3fGeomParam::Sample ceilSamp;
		 iNormals.getExpanded(ceilSamp,Alembic::Abc::ISampleSelector(ceilIndex));
		 ceilVals=ceilSamp.getVals();
		 if(sampSize==ceilVals->size())
		 {
			 Alembic::Abc::N3fArraySamplePtr ceilVal=ceilSamp.getVals();
			 for(size_t i=0;i<sampSize;++i)
			 {
				cout<<"待扩充！"<<endl;
			 }
		 }
		 else
		 {
			 for (size_t i = 0; i < sampSize; ++i)
                {
					outfile<<"vn "<<fixed<<(*sampVal)[i].x<<" "<<(*sampVal)[i].y<<" "<<(*sampVal)[i].z<<endl;
                }
		 }
	 }
	 else
	 {
		 for (size_t i = 0; i < sampSize; ++i)
                {
                   outfile<<"vn "<<fixed<<(*sampVal)[i].x<<" "<<(*sampVal)[i].y<<" "<<(*sampVal)[i].z<<endl;
                }
	 }
	
}


void fillTopology(
	    double iFrame,
		Alembic::AbcGeom::IV2fGeomParam iUVs,
		Alembic::AbcGeom::IN3fGeomParam iNormals,
        Alembic::Abc::Int32ArraySamplePtr iIndices,
        Alembic::Abc::Int32ArraySamplePtr iCounts,string filename)
{
	ofstream outfile;
	outfile.open(filename,ios::app);
	unsigned int numPolys=static_cast<unsigned int>(iCounts->size());
	int *polyCounts=new int[numPolys];

	for(unsigned int i=0;i<numPolys;i++)
	{
		polyCounts[i]=(*iCounts)[i];//每一行都是4个点的信息
	}

	unsigned int numConnects=static_cast<unsigned int>(iIndices->size());
	
	int *polyConnects=new int[numConnects];
	cout<<"num:"<<numConnects<<endl;

	unsigned int facePointIndex=0;
	unsigned int base=0;
	Alembic::Abc::UInt32ArraySamplePtr indexPtr;
	if(iUVs)
	{
		//尝试输出uv索引
		Alembic::AbcCoreAbstract::index_t index,ceilIndex;
		getWeightAndIndex(iFrame,iUVs.getTimeSampling(),iUVs.getNumSamples(),index,ceilIndex);
		Alembic::AbcGeom::IV2fGeomParam::Sample samp;
		iUVs.getIndexed(samp, Alembic::Abc::ISampleSelector(index));
		Alembic::AbcGeom::V2fArraySamplePtr uvPtr = samp.getVals();
		indexPtr = samp.getIndices();
	}
	
	Alembic::Abc::UInt32ArraySamplePtr norPtr;
	if(iNormals)
	{
	//尝试输出法向索引
		  Alembic::AbcCoreAbstract::index_t indexnor,ceilIndexnor;
		  double alpha = getWeightAndIndex(iFrame,
				iNormals.getTimeSampling(), iNormals.getNumSamples(),
				indexnor, ceilIndexnor);
		  if(alpha!=0)
			  cout<<"继续执行该程序"<<endl;
		 Alembic::AbcGeom::IN3fGeomParam::Sample sampnor;
		 iNormals.getIndexed(sampnor,Alembic::Abc::ISampleSelector(indexnor));

		 Alembic::Abc::N3fArraySamplePtr sampVal = sampnor.getVals();
		 norPtr=sampnor.getIndices();
	}


	int uvIndex=0;
	int normalIndex=0;

	for(unsigned int i=0;i<numPolys;i++)
	{
		int curNum=polyCounts[i];
		outfile<<"f"<<" ";

		if(curNum==0)
			continue;

		int normal=normalIndex+curNum-1;

		int startpoint=uvIndex + curNum - 1;
		int nor=curNum-1;
		for (int j = 0; j < curNum; ++j)
		{
                //polyConnects[facePointIndex] = (*iIndices)[base+curNum-j-1];
				outfile<<(*iIndices)[base+curNum-j-1]+1;//点的索引
				//有uv无法向
				if(iUVs && !iNormals)
				{
					outfile<<"/";
					outfile<<(*indexPtr)[startpoint-j]+1<<" ";
				}
				//有法向无uv
				if(!iUVs && iNormals)
				{
					outfile<<"//"<<(*norPtr)[startpoint-j]+1<<" ";
				}
				//有uv有法向
				if(iUVs && iNormals)
				{
					outfile<<"/"<<(*indexPtr)[startpoint-j]+1;
					//outfile<<"/"<<(*norPtr)[normal-nor]+1<<" ";
					outfile<<"/"<<(*norPtr)[startpoint-j]+1<<" ";

				}
				//无法向无uv
				if(!iUVs && !iNormals)
				{
					outfile<<" ";
				}
				uvIndex++;
				nor--;
				normalIndex++;
				
		}
		outfile<<endl;
        base += curNum;
	}


}
