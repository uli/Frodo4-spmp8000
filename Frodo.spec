%define name Frodo
%define version 4.2
%define release 1

Summary: Commodore 64 emulator
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/Emulators
Source: %{name}-V%{version}.tar.gz
URL: http://www.uni-mainz.de/~bauec002/FRMain.html
BuildRoot: %{_tmppath}/%{name}-root
Prefix: %{_prefix}

%description

%prep
%setup -q

%build
cd Src
CFLAGS=${RPM_OPT_FLAGS} CXXFLAGS=${RPM_OPT_FLAGS} ./configure --prefix=%{_prefix}
make

%install
rm -rf ${RPM_BUILD_ROOT}
cd src/Unix
make DESTDIR=${RPM_BUILD_ROOT} install

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc COPYING CHANGES
%{_bindir}/Frodo
%{_bindir}/FrodoPC
%{_bindir}/FrodoSC
%{_bindir}/Frodo_GUI.tcl
%{_datadir}/Frodo/1541 ROM
%{_datadir}/Frodo/Basic ROM
%{_datadir}/Frodo/Char ROM
%{_datadir}/Frodo/Kernal ROM
