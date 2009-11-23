Summary: Persisten Object Storage for C++
Name: POST++
Version: 1.20
Release: 1
Copyright: distributable
Group: Applications/Databases
Source: post-1.20.tar.gz
%description
POST++ provides simple and effective storage for 
application objects. POST++ is based on memory mapping mechanism 
and shadow pages transactions. POST++ eliminates any overhead on 
persistent objects access. Moreover POST++ supports work with several 
storages, storing objects with virtual functions, atomic data file update,
provides high performance memory allocator and optional garbage collector for
implicit deallocation of memory.

%prep
%setup -c

%build
make

%install
make install

%files
/usr/local/post


