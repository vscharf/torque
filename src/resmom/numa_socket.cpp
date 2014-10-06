#include <iostream>
#include <string>
#include <vector>
#include "machine.hpp"
#include <errno.h>
#include <hwloc.h>
#include "pbs_config.h"
#include "pbs_error.h"
#include "log.h"

using namespace std;

#ifdef PENABLE_LINUX_CGROUPS

  Socket::Socket()
    {
    id = 0;
    memory = 0;
    totalThreads = 0;
    totalCores = 0;
    memset(socket_cpuset_string, 0, MAX_CPUSET_SIZE);
    memset(socket_nodeset_string, 0, MAX_NODESET_SIZE);
    }

  Socket::~Socket()
    {
    id = -1;
    }

  /* initializeAMDSocket. 
   * AMD sockets tend to have a hierarcy of socket->numa_node with multimple
   * numa_nodes per socket. This method works on the assumption of this type
   * of architecture
   */
  int Socket::initializeAMDSocket(hwloc_obj_t socket_obj, hwloc_topology_t topology)
    {
    hwloc_obj_t chip_obj;
    hwloc_obj_t prev = NULL;

    this->id = socket_obj->logical_index;
    this->memory = socket_obj->memory.total_memory;
    this->socket_cpuset = socket_obj->allowed_cpuset; 
    this->socket_nodeset = socket_obj->allowed_nodeset;
    hwloc_bitmap_list_snprintf(this->socket_cpuset_string, MAX_CPUSET_SIZE, this->socket_cpuset);
    hwloc_bitmap_list_snprintf(this->socket_nodeset_string, MAX_NODESET_SIZE, this->socket_nodeset);

    this->socket_is_available = true;
    this->totalCores = hwloc_get_nbobjs_inside_cpuset_by_type(topology, this->socket_cpuset, HWLOC_OBJ_CORE);
    this->totalThreads = hwloc_get_nbobjs_inside_cpuset_by_type(topology, this->socket_cpuset, HWLOC_OBJ_PU);
    this->availableCores = this->totalCores;
    this->availableThreads = this->totalThreads;

    /* Get the NUMA Chips for this Socket */
    while ((chip_obj = hwloc_get_next_obj_by_type(topology, HWLOC_OBJ_NODE, prev)) != NULL)
      {
      int is_in_subtree;

      is_in_subtree = hwloc_obj_is_in_subtree(topology, chip_obj, socket_obj);
      if (is_in_subtree)
        {
        Chip new_chip;

        new_chip.initializeChip(chip_obj, topology);
        this->chips.push_back(new_chip);
        }
      prev = chip_obj;
      }

    return(PBSE_NONE);
    }


  /* Intel NUMA sockets have a hierarchy of numa_node->socket. Usually just one socket per
     numa->node. This method works on this assumption for the hardware setup. */
  int Socket::initializeIntelSocket(hwloc_obj_t socket_obj, hwloc_topology_t topology)
    {
    hwloc_obj_t chip_obj;
    hwloc_obj_t prev = NULL;

    this->id = socket_obj->logical_index;
    this->memory = socket_obj->memory.total_memory;
    this->socket_cpuset = socket_obj->allowed_cpuset; 
    this->socket_nodeset = socket_obj->allowed_nodeset;
    hwloc_bitmap_list_snprintf(this->socket_cpuset_string, MAX_CPUSET_SIZE, this->socket_cpuset);
    hwloc_bitmap_list_snprintf(this->socket_nodeset_string, MAX_NODESET_SIZE, this->socket_nodeset);

    this->socket_is_available = true;
    this->totalCores = hwloc_get_nbobjs_inside_cpuset_by_type(topology, this->socket_cpuset, HWLOC_OBJ_CORE);
    this->totalThreads = hwloc_get_nbobjs_inside_cpuset_by_type(topology, this->socket_cpuset, HWLOC_OBJ_PU);
    this->availableCores = this->totalCores;
    this->availableThreads = this->totalThreads;

    /* Get the NUMA Chips for this Socket */
    while ((chip_obj = hwloc_get_next_obj_by_type(topology, HWLOC_OBJ_NODE, prev)) != NULL)
      {
      int is_in_subtree;

      is_in_subtree = hwloc_obj_is_in_subtree(topology, socket_obj, chip_obj);
      if (is_in_subtree)
        {
        Chip new_chip;

        new_chip.initializeChip(chip_obj, topology);
        this->chips.push_back(new_chip);
        }
      prev = chip_obj;

      }

    return(PBSE_NONE);
    }

  /* Socket::initializeNonNUMASocket:
   * If the hardware is not NUMA then there will be no NUMA node
   * on the hardware. This method accounts for no NUMA node and creates
   * a placehodler NUMA Chip class which behaves as if there were
   * a single NUMA Chip on the hardware.
   */
  int Socket::initializeNonNUMASocket(hwloc_obj_t obj, hwloc_topology_t topology)
    {
    /* This is not a NUMA architecture. So we make a NUMA chip as a place holder */
    Chip numaChip;

    numaChip.initializeNonNUMAChip(obj, topology);

    this->totalCores = numaChip.getTotalCores();
    this->totalThreads = numaChip.getTotalThreads();
    this->availableCores = numaChip.getAvailableCores();
    this->availableThreads = numaChip.getAvailableThreads();
    this->socket_is_available = true;

    get_machine_total_memory(topology, &this->memory);    
    this->chips.push_back(numaChip);
    return(PBSE_NONE);
    }

  hwloc_uint64_t Socket::getMemoryInBytes()
    {
    return(this->memory);
    }

  int Socket::getTotalCores()
    {
    return(this->totalCores);
    }

  int Socket::getTotalThreads()
    {
    return(this->totalThreads);
    }

  int Socket::getTotalChips()
    {
    return(this->chips.size());
    }

  int Socket::getAvailableChips()
    {
    int available_chips = 0;
    std::vector<Chip>::iterator chip_iter;

    for (chip_iter = this->chips.begin(); chip_iter < this->chips.end(); chip_iter++)
      {
      if (chip_iter->chipIsAvailable() == true)
        {
        available_chips++;
        }
      }

    return(available_chips);
    }

  int Socket::getAvailableCores()
    {
    return(this->availableCores);
    }

  int Socket::getAvailableThreads()
    {
    return(this->availableThreads);
    }

#endif /* PENABLE_LINUX26_CPUSETS */  