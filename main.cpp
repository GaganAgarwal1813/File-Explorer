//Used for basic input/output stream
#include <stdio.h>
//Used for handling directory files
#include <dirent.h>
//For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <iostream>
#include <filesystem>
#include <fstream>  
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iomanip>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <cstring>
// #include <Windows.h>

#include <sys/ioctl.h> // For window size ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO
#include <bits/stdc++.h>


using namespace std;

#define clearscreen() printf("\033[H\033[J")


static struct termios origSettings, newSettings;





char* croot; // Keeping track of Directory in which code was opened
int wintrack=0; // Keeping track of current terminal window

vector<string>dlist; // List of all directories,files names
int xoffset=0; // Horizontal scrolling coordinate
unsigned currx=1; // X axis for positioning pointer
unsigned curry=1; // Y axis for positioning pointer
unsigned int currdirlistst=0; // Start index of dlist in current window
unsigned int currdirlisted;  // End index of dlist in current window
string curpath; // Current location
char *currpath1; // Current location
stack<string> bstack; // Back stack for left arrow key
stack<string> fstack;  // Forward stack for right arrow key

vector<string> commands; // Stores all the parameters of Command
int totalFiles; // Total no of files in current directory
unsigned int rowsize, colsize; // Current Termial rowsize
int searchflg = 0;

vector<string> searchans;

void opendirect(const char *path); // Defintion of opening a Directory



int isNameMatch(string tosearch, string name) // Comapres the name searched and current name got
{
	if (tosearch == name)
		return 1;
	else
		return 0;
}

void searchall(const char* path,string fname) // Recursively search in all directories
{
	DIR * cdir;
	cdir=opendir(path);
	struct dirent *dpointer;
	if(cdir)
	{
		while((dpointer=readdir(cdir))!=NULL)
		{
			if((string(dpointer->d_name)==".."))
				continue;
			else if((string(dpointer->d_name)=="."))
				continue;
			else
			{
				string fpath=string(path)+"/"+string(dpointer->d_name);
				struct stat sb;
				stat(fpath.c_str(),&sb);
				if((S_ISDIR(sb.st_mode)))
				{
					if(fname==string(dpointer->d_name))
					{
						searchans.push_back(fpath);
					}
					searchall(fpath.c_str(),fname);
				}
				else
				{
					if ((S_ISDIR(sb.st_mode)))
					{
						if(fname==string(dpointer->d_name))
						{
							searchans.push_back(fpath);
						}
						searchall(fpath.c_str(),fname);
					}
					else
					{
						if(isNameMatch(fname,string(dpointer->d_name)))
						{
							searchans.push_back(fpath);
						}
					}
				}
			}
		}
	}

 }


int searchcmd(vector<string>cmdlis) // Search Command Handling
{
	int l=cmdlis.size(); // Clearin the prev data of vector
	searchans.clear();
	if(l==2)
	{
		string fname=cmdlis[1];

		char* currPath_c = new char[PATH_MAX+1];
		getcwd(currPath_c, PATH_MAX+1);
		string cp11 = string(currPath_c);

		searchall(currPath_c,fname);
		if(searchans.size()>=1)
		{
			return 1;
		}
		else
		{
			//cout<<"No search found";
			return 0;
		}
		
	}
	else
		return 0;

}


string cmdpathupdate(string s) // Relative to Absolute Path Conversion
{
	if(s[0]=='.' )
	{
		char* currPath_c = new char[PATH_MAX+1];
		getcwd(currPath_c, PATH_MAX+1);
		string cp11 = string(currPath_c);
		string ans=cp11+s.substr(1,s.length());
		//cout<<ans<<" ";
		return ans;
	}
	else if(s[0]=='/')
	{
		return s;
	}
	else if(s[0]=='~')
	{
		s=s.substr(1,s.length());
		string s11=string(getlogin());
		//cout<<s11<<" ";
		s="/home/"+s11+s;
		
		return s;
	}
	else
		return s;
}



void createdir(string path) // Creating Directory
{
	
	int status = mkdir(path.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
	
	if(status!=-1)
    	{
        cout<<" Directory Created";
    	}
    	else
    	{
    		cout<<"Location can't be accessed";
    	}
}



void delete_file(const char* path) // Deleting File
{
	string path1=string(path);
	remove(path1.c_str());
}

void delete_dir(const char* path) // Deleting Directory
{
	DIR* dvar;
	dvar=opendir(path);
	struct dirent * dpointer;
	

	//cout<<path<<endl;

	if(dvar)
	{
		while((dpointer=readdir(dvar))!=NULL)
		{
			if(string(dpointer->d_name)==".")
			{
				continue;
			}
			else if(string(dpointer->d_name)=="..")
				continue;
			else
			{
				string fpath=string(path)+"/"+string(dpointer->d_name);
				//cout<<fpath<<endl;
				struct stat sb;
				stat(fpath.c_str(), &sb);
				if((S_ISDIR(sb.st_mode)))
				{
					delete_dir(fpath.c_str());

				}
				else
				{
					delete_file(fpath.c_str());
				}			
			}
		}
		closedir(dvar);
		int s=rmdir(path);
		if(s==-1)
			cout<<"Error in removing directory";
	}
	else
		cout<<"No directory present";
}






void rename1(const char* path,string newname) // Rename 
{
	string oldname=string(path);
	
	int lastidx=oldname.find_last_of("/\\");

	newname=cmdpathupdate(newname);
	

	rename(oldname.c_str(), newname.c_str());

}



bool isdirect(const char* dpath) // To check if the given path's last name is a name of directory or not
{
	struct stat sb;
	stat(dpath,&sb);
	if((S_ISDIR(sb.st_mode)))
		return true;
	else
		return false;
}




void copyfile(const char *path, const char *des) // Copy file
{
	char b[1024];
	int fin,fout, nread;
	fin=open(path,O_RDONLY);
	fout=open(des,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	while((nread = read(fin,b,sizeof(b)))>0){
		write(fout,b,nread);
	}
	struct stat sstat;
	stat(path,&sstat);
	struct stat dstat;
	stat(des,&dstat);
	chown(des,sstat.st_uid,sstat.st_gid);
	chmod(des,sstat.st_mode);
}


void copydirect(const char *path, const char *des) // Copy Directory
{
	
	
	createdir(string(des));



	DIR * cdir;
	cdir=opendir(path);
	struct dirent *dpointer;
	
	
	
	if(cdir)
	{
		while((dpointer=readdir(cdir))!=NULL)
		{
			if(string(dpointer->d_name)==".")
			{
				continue;
			}
			else if(string(dpointer->d_name)=="..")
				continue;
			else
			{
				string fpath=string(path)+"/"+string(dpointer->d_name);
				// cout<<fpath;
				struct stat sb;
				stat(fpath.c_str(),&sb);
				string ndestpath=string(des)+"/"+string(dpointer->d_name);
				if((S_ISDIR(sb.st_mode)))
				{
			    		copydirect(fpath.c_str(),ndestpath.c_str());
			    	}
				else
			    {
			    		copyfile(fpath.c_str(),ndestpath.c_str());
				}
			}
		}
	}
	else
	{
		cout<<"Error in copying";
	}

}


void ccomand(vector<string> comdlis)  // FOR HANDLING COPY COMMAND
{
	
	if(comdlis.size()>=3)
	{
		for(int i=1;i<comdlis.size()-1;i++)
		{

			string destpath=comdlis[comdlis.size()-1];

			string prevpath=comdlis[i];

			string fname;
			size_t pos = prevpath.find_last_of("/\\");
			fname = prevpath.substr(pos+1,prevpath.length());

			destpath=destpath+"/"+fname;

			if(isdirect(prevpath.c_str()))
			{
				
				copydirect(prevpath.c_str(),destpath.c_str());
			}
			else
			{
				copyfile(prevpath.c_str(),destpath.c_str());
			}
			
		}

	}
	else
	{
		cout<<"Enter appropriate addresses";
	}
}



void makedirs(vector<string>comdlis) // For handling multiple directories creation in a single command
{
	
	if(comdlis.size()<=2)
	{
		cout<<"Enter appropriate Command";
	}
	else
	{
		string dest=comdlis[comdlis.size()-1];
		for(int i=1;i<comdlis.size()-1;i++)
		{
			string fpath=dest+"/"+comdlis[i];
			

			createdir(fpath);
		}
	}
}


void createfile(vector<string>comdlis) // Create File
{
	string dest=comdlis[comdlis.size()-1];
	for(int i=1;i<comdlis.size()-1;i++)
	{
		string fpath=dest+"/"+comdlis[i];
		ofstream outfile (fpath);
		outfile << "my text here!" << std::endl;
		outfile.close();
	}
}


void cmdstringtovect(string s) // Handling command and converting to vector of string
{

	int flg=0;
	commands.clear();
	// cout<<s;
	// exit(0);
	string temp="";
	int i=0;
	for(;i<s.length() && s[i]!=' ';i++)
	{
		temp=temp+s[i];
	}
	
	commands.push_back((temp)); // Command is pushed
	i++;
	if(commands[0]=="create_dir" || commands[0]=="create_file" || commands[0]=="search")
	{
		flg=1;
	}

	for(;i<s.length();i++)
	{
		string temp="";
		while(i<s.length() && s[i]!=' ')
		{
			if(s[i]=='\\') // MAY BE CAN GET IMPLICITLY HANDLED TO CHECK
			{
				temp=temp+s[i+1];
				i++;
				i++;
			}
			else
			{
				temp=temp+s[i];
				i++;
			}
		}
		
		commands.push_back(cmdpathupdate(temp));
	}
	
	
}



void move(vector<string>comdlis) // Move
{
	for(int i=1;i<comdlis.size()-1;i++)
	{
		string destpath=comdlis[comdlis.size()-1];

		string prevpath=comdlis[i];
			
		string fname;
		size_t pos = prevpath.find_last_of("/\\"); // Last occurance of '/'
		fname = prevpath.substr(pos+1,prevpath.length());

		destpath=destpath+"/"+fname;

		if(isdirect(prevpath.c_str()))
		{
				
			copydirect(prevpath.c_str(),destpath.c_str());
			delete_dir(prevpath.c_str());
		}
		else
		{
			copyfile(prevpath.c_str(),destpath.c_str());
			delete_file(prevpath.c_str());
		}

	}
}


void cmdmode()
{
	printf("%c[%d;%dH", 27, rowsize+1, 1);
	printf("%c[2K", 27);
	cout<<":";
	
	// cout<<"========= COMMAND MODE ============"<<currdirlistst<<" "<<curry<<" "<<currdirlisted;
	while(true)
	{
		char ch;
		string compcmd;
		ch=cin.get();
		while(ch!=10 && ch!=27)
		{
			if(ch==127)
			{
				printf("%c[%d;%dH", 27, rowsize+1, 1);
				printf("%c[2K", 27);
				cout<<":";
				if(compcmd.length()>1)
				{
					compcmd=compcmd.substr(0,compcmd.length()-1);
				}
				else
				{
					compcmd="";
				}
				cout<<compcmd;
			}
			else
			{
				compcmd=compcmd+ch;
				cout<<ch;
			}
			ch=cin.get();
		}
		cmdstringtovect(compcmd); // String to array of string
		if(ch==10)
		{
			if(commands[0]=="copy") // FINE
			{
				ccomand(commands);
				cout<<endl;
				opendirect(".");
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";
				
			}
			else if(commands[0]=="create_dir") //FINE
			{
				makedirs(commands);
				cout<<endl;
				opendirect(".");
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";
			}
			else if(commands[0]=="create_file")
			{
				if(commands.size()<=2)
				{
					cout<<"Enter appropriate Command"<<endl;
				}
				else
				{
					createfile(commands);
					cout<<endl;
				}
				opendirect(".");
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";
			}
			else if(commands[0]=="rename") // FINE 
			{
				if(commands.size()<=2)
				{
					cout<<"Enter appropriate Command"<<endl;
					
				}
				else
				{
					rename1(commands[1].c_str(),commands[2]);
					cout<<endl;
					
				}
				opendirect(".");
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";
			}
			else if(commands[0]=="delete_file") 
			{
				if(commands.size()!=2)
				{
					cout<<"Enter appropriate Command"<<endl;
				}
				else
				{
					delete_file(commands[1].c_str());
					cout<<endl;
				}
				opendirect(".");
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";
			}
			else if(commands[0]=="delete_dir") 
			{
				if(commands.size()!=2)
				{
					cout<<"Enter appropriate Command"<<endl;
				}
				else
				{
					delete_dir(commands[1].c_str());
					cout<<endl;
					
				}
				opendirect(".");
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";
			}
			else if(commands[0]=="q") 
			{
				cout<<endl;
				exit(0);
			}
			else if(commands[0]=="move") 
			{
				if(commands.size()<=2)
				{
					cout<<"Enter appropriate Command"<<endl;
				}
				else
				{
					move(commands);
					cout<<endl;
				}
				opendirect(".");
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";

			}
			else if(commands[0]=="search") 
			{
				if(commands.size()==2)
				{
					int ans=searchcmd(commands);
					if(ans==1)
						cout<<" True"<<endl;
					else
						cout<<" False"<<endl;
					//rowsize=rowsize+2;
					

				}
				else
				{
					cout<<"Enter appropriate Command"<<endl;
				}
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";
			}
			else if(commands[0]=="goto") //  WORKING
			{
				if(commands.size()==2)
				{
					chdir(commands[1].c_str());
					opendirect(".");
				}
				else
				{
					cout<<"Enter appropriate Command"<<endl;
				}
				printf("%c[%d;%dH",27,rowsize+1,1);
				printf("%c[2K", 27);
				cout<<":";
			}
		}
		if(ch==27)
			break;

	}

	
}






string currpathupdate(const char* relativepath)   // Relative to absolute path
{
	char* currPath_c = new char[PATH_MAX+1];
	getcwd(currPath_c, PATH_MAX+1);
	string cp11 = string(currPath_c);
	string convertedrelativepath=string(relativepath);
	cp11.append("/");
	cp11.append(convertedrelativepath);


    return cp11;

  

}


int getprintcount() // Total no of files to be printed in the terminal
{
	struct winsize win;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	rowsize = win.ws_row - 1;
	colsize = win.ws_col;
	if(rowsize<totalFiles)
		return rowsize;
	else
		return totalFiles;
}

void navigate1(const char *dirName,const char *root) // Checking permission of every file and directory
{
	string s="";
    
     struct stat st;
     struct passwd *wd;
     struct group *gp;
     string tm;

     string fpath;
	 
    

     fpath=(currpathupdate(fpath.c_str())).c_str();

     char *path = new char[fpath.length() + 1];



	 strcpy(path, fpath.c_str());

	currpath1=path;

	 string r(root);
	 string file(dirName);
	 r=r+"/"+file;

	 char path111[FILENAME_MAX];
	 strcpy(path111,r.c_str());
	 path111[r.length()]='\0';

	// cout<<path<<" ";
	// strcat(path,"/");
	// strcat(path,dirName);
    
     int ret = stat(path111, &st);




     if(ret == -1) 
     { 

          cout << "open error" << endl; 




          return ; 
     }

     if(S_ISREG(st.st_mode)) 
     {      
     	s=s+"-";
     //    cout << "-";  // Is regular file
    	}
     else if(S_ISDIR(st.st_mode)) 
     {    
     	s=s+"d"; 
          //cout << "d"; // Is directory
     }
     else if(S_ISCHR(st.st_mode)) //  character device file
     {
     	s=s+"c";
          //cout << "c"; 
     }
     else if(S_ISBLK(st.st_mode))   // block device file
     {
     	s=s+"b";
         // cout << "b";
     }
     else if(S_ISLNK(st.st_mode))   // Symbolic Link
     {
     	s=s+"l";
         // cout << "l";
     }
     else if(S_ISFIFO(st.st_mode))  
     {
     	s=s+"f";
         // cout << "f";
     }
     else if(S_ISSOCK(st.st_mode))  // local socket file
     {
     	s=s+"s";
         // cout << "s";
     }
     if (st.st_mode & S_IRUSR)   // Checking read permission  
     {  
     	s=s+"r";
         // cout << "r";
     }
     else      
     {    
     	s=s+"-";         
          //cout << "-";
     }
     if (st.st_mode & S_IWUSR)      // Checking write permission
     {
     	s=s+"w";
          //cout << "w";
     }
     else
     {    
     	s=s+"-";               
      //    cout << "-";
     }
     if (st.st_mode & S_IXUSR)       // Checking executable permission
     {
     	s=s+"x";
          //cout << "x";
     }
     else
     {  
     	s=s+"-";                 
          //cout << "-";
     }
     if (st.st_mode & S_IRGRP)
     {   
     	s=s+"r";   
          //cout << "r";
     }
     else
     {   
     	s=s+"-";                
          //cout << "-";
     }
     if (st.st_mode & S_IWGRP)
     {  
     	s=s+"w";    
          //cout << "w";
     }
     else
     { 
     	s=s+"-";                  
          //cout << "-";
     }
     if (st.st_mode & S_IXGRP)
     { 
     	s=s+"x";     
          //cout << "x";
     }
     else
     {   
     	s=s+"-";                
          //cout << "-";
     }
     if (st.st_mode & S_IROTH)
     {
     	s=s+"r";      
          //cout << "r";
     }
     else
     { 
     	s=s+"-";                  
          //cout << "-";
     }
     if (st.st_mode & S_IWOTH)
     {    
     	s=s+"w";  
          //cout << "w";
     }
     else
     {    
     	s=s+"-";               
          //cout << "-";
     }
     if (st.st_mode & S_IXOTH)
     {   
     	s=s+"x";   
          //cout << "x";
     }
     else 
     {    
     	s=s+"-";              
          //cout << "-";
     }




     //cout << " " << st.st_nlink; // file link count
     s=s+" ";

     wd = getpwuid(st.st_uid); // User Id of file's owner

    
     //cout << " " << wd->pw_name;  // 

     s=s+" "+string(wd->pw_name);

     gp = getgrgid(st.st_gid);  // Name and size pointer 
     
     //cout << " " << gp->gr_name;

     s=s+" "+string(gp->gr_name);

    // double filesize=st.st_size;

    // intmax_t filesize=(intmax_t)st.st_size;

     //cout << " " << filesize/1024<<"KB";

	 float temp=((float)st.st_size) / 1024;
	 string temp1=to_string(temp);
	 int index=temp1.find('.');
	 temp1=temp1.substr(0,index+4);

     s=s+" "+temp1+"KB";

          // Current time

     tm = ctime(&st.st_mtime);



    //  cout << " " << tm.substr(4, 12);

   
       s=s+" "+string(tm.substr(4, 12));


     //cout << " " << dirName << endl;

     s=s+" "+string(dirName)+"\n";




     //cout<<s;

     if(xoffset>=s.length())
     	s="\n";
     else
    		s=s.substr(xoffset,colsize);

     
     cout<<s;

    

          // return 0;

     
     
}

void opendirect(const char *path) // Open Directory
{
	DIR *cdir;

	cdir=opendir(path);
	struct dirent *dircontent;


	if(cdir==NULL)
	{
		cout<<"Unable to open the directory"<<endl;
		exit(1);
	}
	dlist.clear();
	totalFiles=0;
	chdir(path);
	while( (dircontent=readdir(cdir))!=NULL)
	{
		dlist.push_back(string(dircontent->d_name));
		totalFiles++;
		
	}

	sort(dlist.begin(),dlist.end());

	wintrack = 0;
	
	int len=getprintcount();
	clearscreen();

	for(int i=0;i<totalFiles && i<len;i++)
	{
		
		navigate1(dlist[i].c_str(),path);
		
	}

}

void navigate() // Normal Mode all key carrying function
{
	currx=1;
	curry=1;
	currpath1=croot;


	struct termios initialrsettings, newrsettings;
	tcgetattr(fileno(stdin), &initialrsettings);
	//switch to canonical mode and echo mode
	newrsettings = initialrsettings;
	newrsettings.c_lflag &= ~ICANON;
	newrsettings.c_lflag &= ~ECHO;
	
	if (tcsetattr(fileno(stdin), TCSAFLUSH, &newrsettings) != 0)
	{
		fprintf(stderr, "Could not set attributes\n");
	}


	char key;

	while(true)
	{

		printf("%c[%d;%dH", 27, rowsize+1, 1);
		//cout<<"========= NORMAL MODE ============"<<currdirlistst<<" "<<curry<<" "<<currdirlisted;
		cout<<"========= NORMAL MODE ============";
		printf("%c[%d;%dH", 27, curry, currx);  // Positioning the cursor
		key=cin.get();
		
			switch(key) 
			{

				case 65: // UP key
				{
					if(wintrack+curry<=1)
					{
						int a;
					}
					else
					{
						curry--;
						if(curry>0)
						{
							printf("%c[%d;%dH", 27, curry, currx);
						}
						else if(curry<=0 && curry+wintrack>=1)
						{
							clearscreen();
							if(wintrack>0)
								wintrack--;
							printf("%c[%d;%dH", 27, 1, 1);
							for(int i=wintrack;i<=rowsize+wintrack-1;i++)
							{
								navigate1(dlist[i].c_str(),currpath1);
							}
							curry++;
							printf("%c[%d;%dH", 27, curry, currx);
						}
					}	
					break;
				}
				case 66: //Down key
				{
					if(curry+wintrack>=totalFiles)
					{

					}
					else
					{
						curry++;
						if(curry<=rowsize)
						{
							printf("%c[%d;%dH", 27, curry, currx);
						}
						else if(curry>rowsize && curry+wintrack<=totalFiles)
						{
							clearscreen();
							int recordl=getprintcount()-1;
							if(totalFiles>rowsize)
								wintrack++;
							
							printf("%c[%d;%dH", 27, 1, 1);
							for(int i=wintrack;i<=recordl+wintrack;i++)
							{
								navigate1(dlist[i].c_str(),currpath1);
							}
							curry--;
						}
						printf("%c[%d;%dH", 27, curry, currx);
					}
					break;
				}
				case 68:
					//cout << "Left" << endl;  // key left
				{
					if(bstack.empty()==false)
					{
						string top = bstack.top();
						bstack.pop();
						// if(searchflg!=1)
						// {
						// 	fstack.push(string(currpath1));
						// }
						//cout<<currpathupdate(currpath1);
						fstack.push((string((currpath1))));

					

						strcpy(currpath1,top.c_str());
						searchflg=0;

						//cout<<currpath1<<" ";

						opendirect(currpath1);
						curry=1,currx=1;
						printf("%c[%d;%dH", 27, curry, currx);
					}
					// else
					// 	continue;

					break;
				}
				case 67:
				{
    					//cout << "Right" << endl;  // key right
					if(fstack.empty()==false)
					{
						string top=fstack.top();
						fstack.pop();
						bstack.push((string(currpath1)));
						//bstack.push(string(curpath));

						strcpy(currpath1,top.c_str());
						searchflg=0;
						opendirect(currpath1);

						//cout<<"zzzz";
						curry=1,currx=1;
						printf("%c[%d;%dH", 27, curry, currx);
					}
					// else
					// 	continue;
    					break;
    					}




    				case 10: // Enter
    				{
    					string newdir=dlist[curry+wintrack-1];
    					string finalpath;
    					finalpath=string(currpath1)+"/"+newdir;
    					char* path=new char[finalpath.length()+1];
    					strcpy(path,finalpath.c_str());
    					struct stat sb;
					stat(path, &sb);

					if((sb.st_mode & S_IFMT) == S_IFREG)
					{
						int fileOpen=open("/dev/null",O_WRONLY);
						dup2(fileOpen,2);
						close(fileOpen);
						pid_t processID = fork();
						if(processID == 0)
						{
							execlp("xdg-open","xdg-open",path,NULL);
							exit(0);
						}	 
					}
					else if((sb.st_mode & S_IFMT) == S_IFDIR) // Is directory
					{
						searchflg=0;
						curry=1;
						if(newdir==string("."))
						{

						}
						else if(newdir==string(".."))
						{
							// while(!fstack.empty())
							// 	fstack.pop();
							bstack.push((string(currpath1)));

							//bstack.push(string(currpath1));
							string par="../";
							opendirect(par.c_str());


						}
						else
						{
							if(currpath1!=NULL)
							{
								while(!fstack.empty())
									fstack.pop();
								bstack.push((string(currpath1)));
								//bstack.push(string(currpath1));
							}
							// else
							// 	continue;
							currpath1=path;
						}
						//dlist.clear();

						opendirect(currpath1);
					}
					else
					{
						//cout<<currpath1<<" "<<endl;
						cout<<"Cannot open this file type";
					}
    				break;
    				}

    				case 127: // Back Space
    				{
    					bstack.push((string(currpath1)));
					//bstack.push(string(currpath1));
					string par="../";
					opendirect(par.c_str());
					opendirect(".");
					break;
				}
    				
    				case 58: // Switching to command Mode
    				{
    					cmdmode();
    					opendirect(".");
    					break;
    				}
    				case 104: // h 
    				{
    					bstack.push((string(currpath1)));
						string s11=string(getlogin());
						string s="/home/"+s11;
    					opendirect(s.c_str());
    					//return;
    					break;
    				}
    				case 'a':
    				{
    					if(xoffset>0)
    						xoffset--;
    					opendirect(".");
    					break;
    				}
    				case 's':
    				{
    					xoffset++;
    					opendirect(".");
    					break;
    				}
    				case 'q':
    				{
    					exit(0);
    					break;
    				}



					case 'k': // UP key
				{
					if(wintrack+curry<=1)
					{
						int a;
					}
					else
					{
						curry--;
						if(curry>0)
						{
							printf("%c[%d;%dH", 27, curry, currx);
						}
						else if(curry<=0 && curry+wintrack>=1)
						{
							clearscreen();
							if(wintrack>0)
								wintrack--;
							printf("%c[%d;%dH", 27, 1, 1);
							for(int i=wintrack;i<=rowsize+wintrack-1;i++)
							{
								navigate1(dlist[i].c_str(),currpath1);
							}
							curry++;
							printf("%c[%d;%dH", 27, curry, currx);
						}
					}	
					break;
				}
				case 'l': //Down key
				{
					if(curry+wintrack>=totalFiles)
					{

					}
					else
					{
						curry++;
						if(curry<=rowsize)
						{
							printf("%c[%d;%dH", 27, curry, currx);
						}
						else if(curry>rowsize && curry+wintrack<=totalFiles)
						{
							clearscreen();
							int recordl=getprintcount()-1;
							if(totalFiles>rowsize)
								wintrack++;
							
							printf("%c[%d;%dH", 27, 1, 1);
							for(int i=wintrack;i<=recordl+wintrack;i++)
							{
								navigate1(dlist[i].c_str(),currpath1);
							}
							curry--;
						}
						printf("%c[%d;%dH", 27, curry, currx);
					}
					break;
				}
					
			}
		
		
	}

	
	

}


int main(int argc, char *argv[])
{

	if (argc == 1)
	{
		string s1=".";
		string s = currpathupdate(s1.c_str());
		char *path = new char[s.length() + 1];
		strcpy(path, s.c_str());
		croot = path;
		opendirect(s.c_str());
	}
	else if (argc == 2)
	{
		croot = argv[1];
		opendirect(argv[1]);
	}
	else
	{
		cout << "Invalid Argument !!!" << endl;
	}

	//Start Navigating through Command prompt
	navigate();
	
	return 0;
}



