#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "animatePorcess.h"
#include "SampleUtil.h"
using namespace std;

AlembicObjectPtr previsit(AlembicObjectPtr iParentObject)
{
	Alembic::Abc::IObject parent=iParentObject->object();
	const string name=parent.getFullName().c_str();
	cout<<"name="<<name<<endl;
	const size_t numChildren=parent.getNumChildren();
	cout<<"numChildren="<<numChildren<<endl;
	for(size_t i=0;i<numChildren;i++)
	{
		Alembic::Abc::IObject child=parent.getChild(i);
		AlembicObjectPtr childObject=
			previsit(AlembicObjectPtr(new AlembicObject(child)));

		if(childObject)
		{
			//��һ��ִ����
			iParentObject->addChild(childObject);
		}
	}
	return iParentObject;
}
void visit(AlembicObjectPtr iObject)
{
	Alembic::Abc::IObject iObj=iObject->object();

	if(Alembic::AbcGeom::IXform::matches(iObj.getHeader()))
	{
		Alembic::AbcGeom::IXform xform(iObj,Alembic::Abc::kWrapExisting);
		cout<<"IXformģʽƥ��ɹ���"<<endl;

		//����Ĵ����Ƕ�ȡ�ĵ���IXform�е���Ϣ(�ο�prman�еĴ���ProcessXform)
		Alembic::AbcGeom::IXformSchema &xformFS=xform.getSchema();//��ȡ�ö����Ӧ��ģʽ
		TimeSamplingPtr ts=xformFS.getTimeSampling();
		size_t xformSamps=xformFS.getNumSamples();
		//cout<<"numsamples:"<<xformSamps<<endl;
		
		SampleTimeSet sampleTimes;
		GetRelevantSampleTimes(ts,xformSamps,sampleTimes);


		//cout<<"sampleTimes:"<<sampleTimes.size()<<endl;
		bool multiSample=sampleTimes.size()>1;
		std::vector<XformSample> sampleVectors;
		sampleVectors.resize(sampleTimes.size());


		size_t sampleTimeIndex = 0;
		//Alembic::AbcGeom::XformSample *osample=new Alembic::AbcGeom::XformSample();//��ȡ��ģʽ����������
		//xformFS.get(*osample);//��ȡ��������
		for ( SampleTimeSet::iterator I = sampleTimes.begin();
			  I != sampleTimes.end(); ++I, ++sampleTimeIndex )
		{
			ISampleSelector sampleSelector( *I );
			xformFS.get( sampleVectors[sampleTimeIndex], sampleSelector );

		}//forѭ������

		if(xformFS.getInheritsXforms()==false)
			cout<<"prmanAPI�ⲿ�ֲ��ɼ�"<<endl;

		
		 for ( size_t i = 0, e = xformFS.getNumOps(); i < e; ++i )
		 {
			if ( multiSample )
			{ 
				WriteMotionBegin(sampleTimes); 
				//cout<<"multiSampleΪTrue"<<endl;
			}
			for ( size_t j = 0; j < sampleVectors.size(); ++j )
			{
				XformOp &op = sampleVectors[j][i];
				switch ( op.getType() )
				{
				case kScaleOperation:
				{
					V3d value = op.getScale();
					cout<<"����:"<<" ";
					cout<<value.x<<" "<<value.y<<" "<<value.z<<endl;
					cout<<"<scale x="<<"\""<<value.x<<"\" y="<<"\""<<value.y<<"\" z="<<"\""<<value.z<<"\""<<"/>"<<endl;
					break;
				}
				case kTranslateOperation:
				{
					V3d value = op.getTranslate();
					cout<<"ƽ��:"<<" ";
					cout<<value.x<<" "<<value.y<<" "<<value.z<<endl;
					cout<<"<translate x="<<"\""<<value.x<<"\" y="<<"\""<<value.y<<"\" z="<<"\""<<value.z<<"\""<<"/>"<<endl;
					break;
				}

				case kRotateOperation:
				case kRotateXOperation:
				{
					V3d axis = op.getAxis();
					float degrees = op.getAngle();
					cout<<"��ת:"<<" ";
					cout<<axis.x<<" "<<axis.y<<" "<<axis.z<<endl;
					cout<<"��ת�Ƕ�:"<<"  ";
					cout<<degrees<<endl;
					cout<<"<rotate x="<<"\""<<axis.x<<"\" angle="<<"\""<<degrees<<"\""<<"/>"<<endl;
					break;
				}
				case kRotateYOperation:
				{
					V3d axis = op.getAxis();
					float degrees = op.getAngle();
					cout<<"��ת:"<<" ";
					cout<<axis.x<<" "<<axis.y<<" "<<axis.z<<endl;
					cout<<"��ת�Ƕ�:"<<"  ";
					cout<<degrees<<endl;
					cout<<"<rotate y="<<"\""<<axis.y<<"\" angle="<<"\""<<degrees<<"\""<<"/>"<<endl;
					break;
				}
				case kRotateZOperation:
				{
					V3d axis = op.getAxis();
					float degrees = op.getAngle();
					cout<<"��ת:"<<" ";
					cout<<axis.x<<" "<<axis.y<<" "<<axis.z<<endl;
					cout<<"��ת�Ƕ�:"<<"  ";
					cout<<degrees<<endl;
					cout<<"<rotate z="<<"\""<<axis.z<<"\" angle="<<"\""<<degrees<<"\""<<"/>"<<endl;
					break;
				}
				case kMatrixOperation:
				{
					//WriteConcatTransform( op.getMatrix() );
					Alembic::Abc::M44d os_mat=op.getMatrix();
					for(int i=0;i<4;i++)
					{
						for(int j=0;j<4;j++)
						{
							cout<<os_mat[i][j]<<" ";
						}
						cout<<endl;
					}
					break;
				}
				}
			}
    }

	   size_t numChildren=iObject->getNumChildren();
	   if(numChildren==0)
	   {
		 cout<<"numChildrenΪ��"<<endl;
	   }
	   for(size_t i=0;i<numChildren;i++)
	   {
		visit(iObject->getChild(i));
	   }
  }
//ѭ�����������еĽڵ�
	else if(Alembic::AbcGeom::ISubD::matches(iObj.getHeader()))
	{
		Alembic::AbcGeom::ISubD mesh(iObj,Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::ISubDSchema subSche=mesh.getSchema();
		cout<<subSche.getName()<<"  ISubD"<<endl;

	}
	else if(Alembic::AbcGeom::IPolyMesh::matches(iObj.getHeader()))
	{
		Alembic::AbcGeom::IPolyMesh mesh(iObj,Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::IPolyMeshSchema meshSche=mesh.getSchema();
		cout<<meshSche.getName()<<"   IPolyMeshSchema"<<endl;
		cout<<"IPolyMeshģʽƥ��ɹ���"<<endl;

		//�ο�prman������дpolymesh����Ϣ
		Alembic::AbcGeom::IPolyMeshSchema &ps=mesh.getSchema();
		TimeSamplingPtr ts=ps.getTimeSampling();

		SampleTimeSet sampleTimes;
		GetRelevantSampleTimes(ts,ps.getNumSamples(),sampleTimes);

		bool multisample=sampleTimes.size()>1;
		if(multisample)
		{
			WriteMotionBegin(sampleTimes);
		}

		for(SampleTimeSet::iterator iter=sampleTimes.begin();iter!=sampleTimes.end();++iter)
		{
			ISampleSelector sampleSelector(*iter);
			IPolyMeshSchema::Sample sample=ps.getValue(sampleSelector);

			//��õ����Ϣ
			P3fArraySamplePtr point=sample.getPositions();
			//���ļ��Ĳ���
			ofstream outfile;
			outfile.open("transcube.obj");

			//�Ƚ���ǿ������ת����Ȼ���ٻ�ȡ�������
			float32_t *fpoint=(float32_t *)(point->getData());
			outfile<<"v"<<" ";
			int num=0;
			for(int i=0;i<3*(point->size());i++)
			{
				if(num!=0&&num%3==0)
				{
					outfile<<endl;
					outfile<<"v"<<" ";
				}
				outfile<<fixed<<*fpoint<<" ";
				fpoint++;
				num++;
			}
			outfile<<endl;


			//���Ի��uv��Ϣ
			IV2fGeomParam uvParam = ps.getUVsParam();
			setUVs(30,uvParam,"transcube.obj");

			//���Ի�÷�����Ϣ
			IN3fGeomParam nParam = ps.getNormalsParam();
			setPolyNormals(30,nParam,"transcube.obj");
			outfile.close();
			cout<<*(sample.getFaceCounts()->get())<<endl;

			//���Ի�����������Ϣ
			fillTopology(sample.getFaceIndices(),sample.getFaceCounts(),"transcube.obj");

			//���Ի�÷����������Ϣ

	
	

		}//forѭ������
	}//ifѭ������

	else if(Alembic::AbcGeom::ICamera::matches(iObj.getHeader()))
	{
		Alembic::AbcGeom::ICamera cam(iObj,Alembic::Abc::kWrapExisting);

		Alembic::AbcGeom::ICameraSchema camSche=cam.getSchema();
		camSche=cam.getSchema();
		cout<<camSche.getName()<<"  ICamera"<<endl;
		Alembic::AbcGeom::CameraSample *camsample=new Alembic::AbcGeom::CameraSample();//��ȡ��ģʽ����������
		camsample=new Alembic::AbcGeom::CameraSample();
		camSche.get(*camsample);//��ȡ��������


		cout<<"ICameraģʽƥ��ɹ���"<<endl;
	}
	else if(Alembic::AbcGeom::ICurves::matches(iObj.getHeader()))
	{
		Alembic::AbcGeom::ICurves curves(iObj,Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::ICurvesSchema curveSche=curves.getSchema();
		cout<<curveSche.getName()<<"  ICurves"<<endl;
		bool isConstant=curves.getSchema().isConstant();
		Alembic::AbcGeom::ICurvesSchema::Sample *cursamp=new Alembic::AbcGeom::ICurvesSchema::Sample();
		curveSche.get(*cursamp);
		//cout<<"���ߵ�������"<<cursamp->getNumCurves()<<endl;

		
	}
	else if(Alembic::AbcGeom::INuPatch::matches(iObj.getHeader()))
	{
		Alembic::AbcGeom::INuPatch nurbs(iObj,Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::INuPatchSchema inuSche=nurbs.getSchema();
		cout<<inuSche.getName()<<"  INuPatch"<<endl;
		cout<<"INuPatchģʽƥ��ɹ���"<<endl;

	}
	else if(Alembic::AbcGeom::IPoints::matches(iObj.getHeader()))
	{
		Alembic::AbcGeom::IPoints pts(iObj,Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::IPointsSchema ipoSche=pts.getSchema();
		cout<<ipoSche.getName()<<"  IPoints"<<endl;
	}

}
