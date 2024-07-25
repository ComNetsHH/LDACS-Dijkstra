# The LDACS Dijkstra implements the Dijkstra's shortest path routing algorithm.
# Copyright (C) 2024  Musab Ahmed, Sohini Maji, Institute of Communication Networks, Hamburg University of Technology, Hamburg, Germany

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

###
# This file is responsible for providing the building the toolchain .
###

build-debug:	
	cd ../../; \
	echo -e "\ntracebasedapp"; \
	cd intairnet-tracebasedapp/src; opp_makemake -f -s --deep -O out -KINET4_PROJ=../../inet4 -DINET_IMPORT -I../../inet4 -I. -I../../inet4/src -L../../inet4/src -lINET_dbg; make -j8 MODE=debug; cd ../..; \
	echo -e "\nTDMA"; \
	cd tdma/tdma/src; opp_makemake --make-so -f --deep -KINET_PROJ=../../../inet4 -DINET_IMPORT -I../../../inet4/src -L../../../inet4/src -lINET_dbg; make -j8 MODE=debug; cd ../../..; \
	echo -e "\ndspr"; \
	cd dspr/src; opp_makemake -f --deep -O out -KINET4_PROJ=../../inet4 -DINET_IMPORT -I../../inet4 -I../../intairnet-tracebasedapp/src -I../../tdma/tdma/src -I. -I../../inet4/src -L../../inet4/src -L../../intairnet-tracebasedapp/out/gcc-debug/src/ -L../../tdma/tdma/out/gcc-debug/src/ -lINET_dbg -lintairnet-tracebasedapp_dbg -ltdma_dbg; make -j$(NUM_CPUS) MODE=debug; \

build-release:	
	cd ../../; \
	echo -e "\ntracebasedapp"; \
	cd intairnet-tracebasedapp/src; opp_makemake -f -s --deep -O out -KINET4_PROJ=../../inet4 -DINET_IMPORT -I../../inet4 -I. -I../../inet4/src -L../../inet4/src -lINET; make -j8 MODE=release; cd ../..; \
	echo -e "\nTDMA"; \
	cd tdma/tdma/src; opp_makemake --make-so -f --deep -KINET_PROJ=../../../inet4 -DINET_IMPORT -I../../../inet4/src -L../../../inet4/src -lINET; make -j8 MODE=release; cd ../../..; \
	echo -e "\ndspr"; \
	cd dspr/src; opp_makemake -f --deep -O out -KINET4_PROJ=../../inet4 -DINET_IMPORT -I../../inet4 -I../../intairnet-tracebasedapp/src -I../../tdma/tdma/src -I. -I../../inet4/src -L../../inet4/src -L../../intairnet-tracebasedapp/out/gcc-release/src/ -L../../tdma/tdma/out/gcc-release/src/ -lINET -lintairnet-tracebasedapp -ltdma; make -j4 MODE=release