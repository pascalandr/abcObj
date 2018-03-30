#include "animatePorcess.h"
#include "global.h"
#include <fstream>
#include <iomanip>
#include <Alembic/AbcGeom/All.h>

using namespace std;

//�ο�prman
void GetRelevantSampleTimes(TimeSamplingPtr timeSampling,
							size_t numSamples, SampleTimeSet &output)
{
	if(numSamples<2)
	{
		output.insert(0.0);
		return;
	}
	chrono_t frameTime=frame/24.0;//�൱����һ֡���ڵ�ʱ��
	chrono_t shutterOpenTime=(frame+0.0)/24.0;
	chrono_t shutterCloseTime=(frame+0.5)/24.0;
	std::pair<index_t,chrono_t> shutterOpenFloor=timeSampling->getFloorIndex(shutterOpenTime,numSamples);//ȡ����
	std::pair<index_t,chrono_t> shutterCloseCeil=timeSampling->getCeilIndex(shutterCloseTime,numSamples);//ȡ����

	//static const chrono_t epsilon=1.0/10000.0;
	//���ڶ��������Ƿ���������Ҫ�ģ��µ��������ȷ��������������������ʹ�á�
	//if(shutterOpenFloor.first<shutterCloseCeil.first)
	//{
	//	//������ǵ��������ڿ���ʱ�䣬�뿴��һ������ʱ��
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
		output.insert(timeSampling->getSampleTime(i));//����κ�������ʱ��
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

	//�ҵ�ʱ��С�ڻ���ڸ���ʱ��������Ч������ʹ�����������Ч
	//�����С����ʱ�����iFrame���򷵻�0
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
		cout<<"uv��Ч"<<endl;
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
		cout<<"normal��Ч��"<<endl;
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
				cout<<"�����䣡"<<endl;
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
		polyCounts[i]=(*iCounts)[i];//ÿһ�ж���4�������Ϣ
	}

	unsigned int numConnects=static_cast<unsigned int>(iIndices->size());
	
	int *polyConnects=new int[numConnects];
	cout<<"num:"<<numConnects<<endl;

	unsigned int facePointIndex=0;
	unsigned int base=0;
	Alembic::Abc::UInt32ArraySamplePtr indexPtr;
	if(iUVs)
	{
		//�������uv����
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
	//���������������
		  Alembic::AbcCoreAbstract::index_t indexnor,ceilIndexnor;
		  double alpha = getWeightAndIndex(iFrame,
				iNormals.getTimeSampling(), iNormals.getNumSamples(),
				indexnor, ceilIndexnor);
		  if(alpha!=0)
			  cout<<"����ִ�иó���"<<endl;
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
				outfile<<(*iIndices)[base+curNum-j-1]+1;//�������
				//��uv�޷���
				if(iUVs && !iNormals)
				{
					outfile<<"/";
					outfile<<(*indexPtr)[startpoint-j]+1<<" ";
				}
				//�з�����uv
				if(!iUVs && iNormals)
				{
					outfile<<"//"<<(*norPtr)[startpoint-j]+1<<" ";
				}
				//��uv�з���
				if(iUVs && iNormals)
				{
					outfile<<"/"<<(*indexPtr)[startpoint-j]+1;
					//outfile<<"/"<<(*norPtr)[normal-nor]+1<<" ";
					outfile<<"/"<<(*norPtr)[startpoint-j]+1<<" ";

				}
				//�޷�����uv
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
