// Diplom.cpp : Defines the entry point for the console application.
//
#include "iostream"
#include "conio.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdafx.h"
#include "io.h"
#include <fstream>
#include <strstream>
#include "wavIO.h"
#include "FftWrapper.h"
#include <math.h>
#include <vector>
#include <string>
#include "BmpDump.h"
#include <iostream>


using namespace std;
#define M_2PI 6.283185307179586476925286766559

//--------------------------------------------------------------------------

//Класс, занимающийся оконным массива
class Window {
private:
	vector<float> window;

public:
	Window(float size)
	{
		float invLength = 1.0 / size ;
		for (int i = 0; i < size ; i++)
			window.push_back( sqrt( float(1.0-cos((float) M_2PI* (i+0.5)*invLength) ) ) );
	}
	
	float get(int i) { return window[i]; }
	int getWindowSize() { return window.size(); }
};

//--------------------------------------------------------------------------
// Класс, занимающийся загружаемым файлом.
class Data {
private:
	vector<float> values;
public:
	void set(short int *input, int size)
	{
		size = size / sizeof(short int);
		for (int i=0; i<size; i++)
			values.push_back(float(input[i])/float(32768));
	}
 
 float get(int i)
 {
	 if (i<values.size() && i>0)
		 return values[i];
	 else
		 return -1;
 }
 
 int getSize() { return values.size(); }

 void print() 
 {
	 for (int i=0; i <values.size(); i++)
		 cout << values[i] << "\n ";
 }

};

//--------------------------------------------------------------------------

//Класс, занимаюющийся спектрограммой
class Spectrogram {
private:
	vector<vector<Cmplx>> s;
	int sizeY;

public:
	void addSpectr(vector<Cmplx> a, int size) //добавление вертикальной полоски спектра в спектрограмму
	{
		vector<Cmplx> b;
		for (int i=0; i<size; i++)
			b.push_back(a[i]);
		s.push_back(b);
		sizeY = size;
	}
 
	void clear()
	{ 
		int t = s.size();
		for(int z=0; z< t;  z++)
			s[z].clear();
		s.clear();
	}

	float getModule(int x, int y) //преобразование комплексного числа в натуральное
	{
		return sqrt( s[x][y].re*s[x][y].re + s[x][y].im*s[x][y].im);
	}
   
	int getSizeX() { return s.size(); }
 
	int getSizeY() { return sizeY; }

};


//--------------------------------------------------------------------------
vector <float> Furie(string myPath)
{ // open the wav file

	int sec =3;
	int a=32;
	string path = myPath; 

	WavFileForIO myWav ( path.c_str() );//


	myWav.myDataSize = myWav.mySampleRate*sec ;

	myWav.save();
	
	Data data;
	data.set( (short int*) myWav.myData, myWav.GetMyDataSize()); // Извлечение из звука массива

	Window win(2048);
	Fft fft;
 	fft.SetMode(FftModes::FFT_REAL, win.getWindowSize());//Инициализация ФФТ
    Spectrogram spectrogram;
    spectrogram.clear();
 
	vector<float> fftInput;
    fftInput.clear();
    vector<Cmplx> fftOut;
    fftOut.clear();
	int c= data.getSize();
	int b=win.getWindowSize();
    fftOut.resize(win.getWindowSize());
 
	float overlap = 1.0/4.0; //нахлёст, с которым мы берём части массива для применения ФФТ
   	int step = win.getWindowSize() * overlap;// шаг, с которым мы идём по звуковому массиву

	for (int j=0; j<data.getSize()-win.getWindowSize(); j+= step)
	{
		fftInput.clear();
		for (int i=0; i<win.getWindowSize(); i++)
		{
			fftInput.push_back( data.get(j+i) * win.get(i) );// Домножение звукового массива на окно
		}
		fft.FftReal(&fftInput[0], &fftOut[0]);//Использовать -прямое ффт
		spectrogram.addSpectr(fftOut, win.getWindowSize()/2+1);// Добовление спектра в спектрограмму

	}
//--------------------------------------------------------------------------

//Отладка, визуализация спректрограммы
   getBmpDump().SetLogFilename("Z:\\Sp_s\\Spectrogram.bmp");
   vector<float> modulasSpectr;
   modulasSpectr.clear();
   vector<float> workSpectr;
   float z = 1.0;
   vector<float> granica;
   granica.clear();
   workSpectr.clear();


 
	int tmp = spectrogram.getSizeX();
	for (int i=0; i<spectrogram.getSizeX(); i++)
	{
		modulasSpectr.clear();
		for (int j=0; j<spectrogram.getSizeY(); j++)
			modulasSpectr.push_back(spectrogram.getModule(i,j));
			
		
		for(int u=0; u < (int)modulasSpectr.size(); u++)
			workSpectr.push_back(modulasSpectr[u]);
		
		
		getBmpDump().AddColumn<float>(&modulasSpectr[0],modulasSpectr.size());
	
	
	}


	for(int i=0; i<(int)modulasSpectr.size();i++)
		granica.push_back(z);

	getBmpDump().AddColumn<float>(&granica[0], granica.size());

	float maximum;
	maximum=0;
	for(int t=0; t<workSpectr.size(); t++)
	{
		if(workSpectr[t]>maximum)
			maximum=workSpectr[t];
	}

	for(int a=0; a < workSpectr.size(); a++)
	{
		workSpectr[a]=workSpectr[a]/maximum;
	
	}



//Нормализация, выделение ненужных частот.









	return workSpectr;

	
}
float Distance( string newPath, vector <float> RightSpectr)
{
	cout<<RightSpectr.size();
	vector<float> spectrNew;
	float distance=0;
	spectrNew = Furie(newPath);
	float timed;
	for (int j=0 ;j<RightSpectr.size(); j++)///??????!!!!!
	{
		timed = RightSpectr[j]-spectrNew[j];
		distance = distance + timed*timed;
	}
	return distance;
}




int main ()
//(int argc, char* argv[])
{
	struct _finddata_t c_file;
	long hFile;
	char buffer1[128];
	string myRightPath = "Z:\\Example\\sss.wav";
	vector<float> spectr;
	vector<float> RightSpectr;

	fstream filestr;
	FILE * pFile;
	pFile = fopen ("Z:\\Statistics\\statistics.txt","w");// Создание файла со статистикой
	
	
	if (pFile!=NULL)
	{
		fclose (pFile);
	}
	
	string DerectoryPath = "Z:\\Example\\" ;
    string Derectory;	  
	
	RightSpectr = Furie(myRightPath);
 


	hFile = _findfirst( "Z:\\Example\\*.wav", &c_file );// Построчное чтение и запись статистики в файл
	
	ifstream fin("Z:\\Example\\info.txt");
    filestr.open ("Z:\\Statistics\\statistics.txt") ;
	
	float cherta;
	cherta = 380.0;
	float MaxTrue = 0;
	float MinTrue =  1000000.0;
	float MaxFalse = 0;
	float MinFalse = 1000000.0;
	
	int otvet = 0;
		while (!fin.eof())
	{

		Derectory = DerectoryPath;
		fin.getline(buffer1, 128);
		string fname(c_file.name);
		Derectory+=fname;
		
		float result = Distance(Derectory, RightSpectr); // Подсчёт расстояния между спектрами
		cout<<Derectory<<endl;
		if(result< cherta)
		{
			otvet = 1;
			if(result < MinTrue)
				MinTrue = result;

			if(result > MaxTrue)
				MaxTrue = result;
		}
			
		else
		{
			otvet = 0;
			if(result < MinFalse)
				MinFalse = result;

			if(result > MaxFalse)
				MaxFalse = result;

		}





		filestr<<buffer1<<"\t"<<"result="<<"\t"<<result<<"\t"<<otvet<<"\n";
		_findnext( hFile, &c_file );
	}
	filestr.close();
	
 	
	int a;


	float porog;

	porog = (MaxTrue*MaxFalse + MinTrue*MinFalse)/( (MaxFalse-MinFalse) + (MaxTrue - MinTrue ) ); 


	cout<<"MinTrue="<<MinTrue<<endl<<"MaxTrue="<<MaxTrue<<endl<<"MinFalse="<<MinFalse<<"MaxFalse="<<MaxFalse<<endl<<endl<<endl;
    cout<<"porog"<<porog;
	cout<<"End of work"<<endl<<">>";
	cin>>a;

}
