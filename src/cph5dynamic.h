////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Ball Aerospace & Technologies Corp. All Rights Reserved.
//
// This program is free software; you can modify and/or redistribute it under
// the terms found in the accompanying LICENSE.txt file.
////////////////////////////////////////////////////////////////////////////////

#ifndef CPH5DYNAMIC_H
#define CPH5DYNAMIC_H

#include "cph5.h"

class CPH5Dynamic {
    
    typedef std::vector<hsize_t> hslist;
    
    // dataSetAdder needs to be a struct instead of a function so that we can
    // partially specialize for 0. It can be used like a regular function though.
    template<typename T, const int X>
    struct dataSetAdder {
        dataSetAdder(CPH5Group &group, H5::DataType &type, std::string name,
                     hslist dims, hslist maxdims, hslist chunks) {
            CPH5Dataset<T,X> *p = new CPH5Dataset<T,X>(&group, name, type);
            p->setDimensions(dims.data(), maxdims.data());
            if (!chunks.empty()) {
                p->setChunkSize(chunks.data());
            }
            group.registerExternalChild(p);
        }
    };
    
    template<typename T>
    struct dataSetAdder<T,0> {
        dataSetAdder(CPH5Group &group, H5::DataType &type, std::string name,
                     hslist dims, hslist maxdims, hslist chunks) {
            CPH5Dataset<T,0> *p = new CPH5Dataset<T,0>(&group, name, type);
            group.registerExternalChild(p);
        }
    };
    
    template<typename T>
    static void dataSetPass(int rank,
                            CPH5Group &group,
                            H5::DataType &type,
                            std::string name,
                            hslist
                            dims,
                            hslist maxdims,
                            hslist chunks) {
        switch (rank) {
            case 0:  { dataSetAdder<T,0>(group, type, name, dims, maxdims, chunks);  break; }
            case 1:  { dataSetAdder<T,1>(group, type, name, dims, maxdims, chunks);  break; }
            case 2:  { dataSetAdder<T,2>(group, type, name, dims, maxdims, chunks);  break; }
            case 3:  { dataSetAdder<T,3>(group, type, name, dims, maxdims, chunks);  break; }
            case 4:  { dataSetAdder<T,4>(group, type, name, dims, maxdims, chunks);  break; }
            case 5:  { dataSetAdder<T,5>(group, type, name, dims, maxdims, chunks);  break; }
            case 6:  { dataSetAdder<T,6>(group, type, name, dims, maxdims, chunks);  break; }
            case 7:  { dataSetAdder<T,7>(group, type, name, dims, maxdims, chunks);  break; }
            case 8:  { dataSetAdder<T,8>(group, type, name, dims, maxdims, chunks);  break; }
            case 9:  { dataSetAdder<T,9>(group, type, name, dims, maxdims, chunks);  break; }
            case 10: { dataSetAdder<T,10>(group, type, name, dims, maxdims, chunks); break; }
            case 11: { dataSetAdder<T,11>(group, type, name, dims, maxdims, chunks); break; }
            case 12: { dataSetAdder<T,12>(group, type, name, dims, maxdims, chunks); break; }
            case 13: { dataSetAdder<T,13>(group, type, name, dims, maxdims, chunks); break; }
            case 14: { dataSetAdder<T,14>(group, type, name, dims, maxdims, chunks); break; }
            case 15: { dataSetAdder<T,15>(group, type, name, dims, maxdims, chunks); break; }
            case 16: { dataSetAdder<T,16>(group, type, name, dims, maxdims, chunks); break; }
            case 17: { dataSetAdder<T,17>(group, type, name, dims, maxdims, chunks); break; }
            case 18: { dataSetAdder<T,18>(group, type, name, dims, maxdims, chunks); break; }
            case 19: { dataSetAdder<T,19>(group, type, name, dims, maxdims, chunks); break; }
            case 20: { dataSetAdder<T,20>(group, type, name, dims, maxdims, chunks); break; }
            case 21: { dataSetAdder<T,21>(group, type, name, dims, maxdims, chunks); break; }
            case 22: { dataSetAdder<T,22>(group, type, name, dims, maxdims, chunks); break; }
            case 23: { dataSetAdder<T,23>(group, type, name, dims, maxdims, chunks); break; }
            case 24: { dataSetAdder<T,24>(group, type, name, dims, maxdims, chunks); break; }
            case 25: { dataSetAdder<T,25>(group, type, name, dims, maxdims, chunks); break; }
            case 26: { dataSetAdder<T,26>(group, type, name, dims, maxdims, chunks); break; }
            case 27: { dataSetAdder<T,27>(group, type, name, dims, maxdims, chunks); break; }
            case 28: { dataSetAdder<T,28>(group, type, name, dims, maxdims, chunks); break; }
            case 29: { dataSetAdder<T,29>(group, type, name, dims, maxdims, chunks); break; }
            case 30: { dataSetAdder<T,30>(group, type, name, dims, maxdims, chunks); break; }
            case 31: { dataSetAdder<T,31>(group, type, name, dims, maxdims, chunks); break; }
            case 32: { dataSetAdder<T,32>(group, type, name, dims, maxdims, chunks); break; }
            default:
                throw "Should never happen";
                break;
        }
    }
    
    static void addDataset(CPH5Group &group,
                           H5::DataType &type,
                           int rank,
                           std::string dsetname,
                           hslist dims,
                           hslist maxdims,
                           hslist chunks) {
        bool isSigned = (H5Tget_sign(type.getId()) == H5T_SGN_2);
        int tClass = type.getClass();
        int size = H5Tget_size(type.getId());
        
        if (tClass == H5T_FLOAT) {
            if (size == sizeof(float)) {
                dataSetPass<float>(rank, group, type, dsetname, dims, maxdims, chunks);
                return;
            } else if (size == sizeof(double)) {
                dataSetPass<double>(rank, group, type, dsetname, dims, maxdims, chunks);
                return;
            }
            throw "Should never happen 2";
        } else if (tClass == H5T_INTEGER) {
            if (isSigned) {
                if (size == sizeof(int8_t)) {
                    dataSetPass<int8_t>(rank, group, type, dsetname, dims, maxdims, chunks);
                    return;
                } else if (size == sizeof(int16_t)) {
                    dataSetPass<int16_t>(rank, group, type, dsetname, dims, maxdims, chunks);
                    return;
                } else if (size == sizeof(int32_t)) {
                    dataSetPass<int32_t>(rank, group, type, dsetname, dims, maxdims, chunks);
                    return;
                } else if (size == sizeof(int64_t)) {
                    dataSetPass<int64_t>(rank, group, type, dsetname, dims, maxdims, chunks);
                    return;
                }
            } else {
                if (size == sizeof(uint8_t)) {
                    dataSetPass<uint8_t>(rank, group, type, dsetname, dims, maxdims, chunks);
                    return;
                } else if (size == sizeof(uint16_t)) {
                    dataSetPass<uint16_t>(rank, group, type, dsetname, dims, maxdims, chunks);
                    return;
                } else if (size == sizeof(uint32_t)) {
                    dataSetPass<uint32_t>(rank, group, type, dsetname, dims, maxdims, chunks);
                    return;
                } else if (size == sizeof(uint64_t)) {
                    dataSetPass<uint64_t>(rank, group, type, dsetname, dims, maxdims, chunks);
                    return;
                }
            }
        } else {
            throw "Should never happen 3";
        }
        
    }
    
    template<const int X>
    static CPH5Dataset<CPH5CompType, 0> *dataSetCompAdder(CPH5Group &group,
                                                          H5::CompType &type,
                                                          std::string dsetname,
                                                          hslist dims,
                                                          hslist maxdims,
                                                          hslist chunks)
    {
        CPH5Dataset<CPH5CompType,X> *p = new CPH5Dataset<CPH5CompType,X>(&group, dsetname, type);
        p->setDimensions(dims.data(), maxdims.data());
        if (!chunks.empty()) {
            p->setChunkSize(chunks.data());
        }
        group.registerExternalChild(p);
        return p->getScalarRef();
    }
    
    static CPH5Dataset<CPH5CompType, 0> *dataSetCompAdderScalar(CPH5Group &group,
                                                                H5::CompType &type,
                                                                std::string dsetname)
    {
        CPH5Dataset<CPH5CompType,0> *p = new CPH5Dataset<CPH5CompType,0>(&group, dsetname, type);
        group.registerExternalChild(p);
        return p->getScalarRef();
    }
    
    static CPH5Dataset<CPH5CompType, 0> *dataSetCompPass(int rank,
                                                         CPH5Group &group,
                                                         H5::CompType &type,
                                                         std::string dsetname,
                                                         hslist dims,
                                                         hslist maxdims,
                                                         hslist chunks)
    {
        switch (rank) {
            case 0:  return dataSetCompAdderScalar(group, type, dsetname); break;
            case 1:  return dataSetCompAdder<1> (group, type, dsetname, dims, maxdims, chunks); break;
            case 2:  return dataSetCompAdder<2> (group, type, dsetname, dims, maxdims, chunks); break;
            case 3:  return dataSetCompAdder<3> (group, type, dsetname, dims, maxdims, chunks); break;
            case 4:  return dataSetCompAdder<4> (group, type, dsetname, dims, maxdims, chunks); break;
            case 5:  return dataSetCompAdder<5> (group, type, dsetname, dims, maxdims, chunks); break;
            case 6:  return dataSetCompAdder<6> (group, type, dsetname, dims, maxdims, chunks); break;
            case 7:  return dataSetCompAdder<7> (group, type, dsetname, dims, maxdims, chunks); break;
            case 8:  return dataSetCompAdder<8> (group, type, dsetname, dims, maxdims, chunks); break;
            case 9:  return dataSetCompAdder<9> (group, type, dsetname, dims, maxdims, chunks); break;
            case 10: return dataSetCompAdder<10>(group, type, dsetname, dims, maxdims, chunks); break;
            case 11: return dataSetCompAdder<11>(group, type, dsetname, dims, maxdims, chunks); break;
            case 12: return dataSetCompAdder<12>(group, type, dsetname, dims, maxdims, chunks); break;
            case 13: return dataSetCompAdder<13>(group, type, dsetname, dims, maxdims, chunks); break;
            case 14: return dataSetCompAdder<14>(group, type, dsetname, dims, maxdims, chunks); break;
            case 15: return dataSetCompAdder<15>(group, type, dsetname, dims, maxdims, chunks); break;
            case 16: return dataSetCompAdder<16>(group, type, dsetname, dims, maxdims, chunks); break;
            case 17: return dataSetCompAdder<17>(group, type, dsetname, dims, maxdims, chunks); break;
            case 18: return dataSetCompAdder<18>(group, type, dsetname, dims, maxdims, chunks); break;
            case 19: return dataSetCompAdder<19>(group, type, dsetname, dims, maxdims, chunks); break;
            case 20: return dataSetCompAdder<20>(group, type, dsetname, dims, maxdims, chunks); break;
            case 21: return dataSetCompAdder<21>(group, type, dsetname, dims, maxdims, chunks); break;
            case 22: return dataSetCompAdder<22>(group, type, dsetname, dims, maxdims, chunks); break;
            case 23: return dataSetCompAdder<23>(group, type, dsetname, dims, maxdims, chunks); break;
            case 24: return dataSetCompAdder<24>(group, type, dsetname, dims, maxdims, chunks); break;
            case 25: return dataSetCompAdder<25>(group, type, dsetname, dims, maxdims, chunks); break;
            case 26: return dataSetCompAdder<26>(group, type, dsetname, dims, maxdims, chunks); break;
            case 27: return dataSetCompAdder<27>(group, type, dsetname, dims, maxdims, chunks); break;
            case 28: return dataSetCompAdder<28>(group, type, dsetname, dims, maxdims, chunks); break;
            case 29: return dataSetCompAdder<29>(group, type, dsetname, dims, maxdims, chunks); break;
            case 30: return dataSetCompAdder<30>(group, type, dsetname, dims, maxdims, chunks); break;
            case 31: return dataSetCompAdder<31>(group, type, dsetname, dims, maxdims, chunks); break;
            case 32: return dataSetCompAdder<32>(group, type, dsetname, dims, maxdims, chunks); break;
            default:
                throw "BAD";
                break;
        }
    }
    
    template<typename T> inline static H5T_sign_t getH5Sign() {
        return H5T_SGN_ERROR;
    }

    
    template<typename T>
    inline static bool memberAdderIf(const H5::DataType &h5type, CPH5CompType *fill, std::string name) {
        if (getH5Sign<T>() == H5Tget_sign(h5type.getId()) &&
            h5type.getSize() == sizeof(T)) {
            auto *pAdd = new CPH5CompMember<T>(fill, name, h5type);
            fill->registerExternalMember(pAdd);
            return true;
        }
        return false;
    }
    

    
    
    // The Array stuff
    template<typename T>
    inline static bool arrayAddIf(H5::DataType &h5type, CPH5CompType *fill, std::string name, int nElements) {
        if (getH5Sign<T>() == H5Tget_sign(h5type.getId()) &&
            h5type.getSize() == sizeof(T)) {
            auto *pAdd = new CPH5CompMemberArrayCommon<T, IS_NOT_DERIVED>(fill, name, h5type, nElements);
            fill->registerExternalMember(pAdd);
            return true;
        }
        return false;
    }
    

    
    
    //declared below
    inline static void recurseComptype(const H5::CompType &h5type, CPH5CompType *fill) ;
    
    static void doDataset(CPH5Group &cph5Top, H5::Group &topGroup, std::string dsetname) {
        H5::DataSet dset = topGroup.openDataSet(dsetname);
        H5::DataSpace space = dset.getSpace();
        int rank = space.getSimpleExtentNdims();
        
        // Get the dimensions
        hslist dims;
        dims.resize(rank);
        hslist maxdims;
        maxdims.resize(rank);
        space.getSimpleExtentDims(dims.data(), maxdims.data());
        // Get chunk size
        H5::DSetCreatPropList cparms = dset.getCreatePlist();
        hslist chunks;
        if (cparms.getLayout() == H5D_CHUNKED) {
            chunks.resize(rank);
            cparms.getChunk(rank, chunks.data());
        }
        
        H5::DataType type = dset.getDataType();
        if (type.getClass() == H5T_INTEGER || type.getClass() == H5T_FLOAT) {
            // Dataset of regular types. Add it
            addDataset(cph5Top, type, rank, dsetname, dims, maxdims, chunks);
        } else if (type.getClass() == H5T_COMPOUND) {
            // Do the big compound thing
            // Create the dataset first with the type read from the file,
            // then get the scalar rank dataset (which is also the compound type)
            // and do the recursive type reconstruction on that.
            H5::CompType compType = dset.getCompType();
            CPH5Dataset<CPH5CompType, 0> *pScalarRank = dataSetCompPass(rank,
                                                                        cph5Top,
                                                                        compType,
                                                                        dsetname,
                                                                        dims,
                                                                        maxdims,
                                                                        chunks);
            // At this point, scalarRank IS a compound type object that is also a child
            // of the CPH5 tree. Now, recursively generate the compound type and add it all
            // to scalarRank.
            recurseComptype(compType, dynamic_cast<CPH5CompType*>(pScalarRank));
        }
    }
    
    static void recurseGroups(CPH5Group &cph5Top, H5::Group &topGroup) {
        std::vector<std::string> groupNames;
        
        // Get all the group names
        hsize_t nobj;
        const int BUFSIZE = 1024;
        char nameBuf[BUFSIZE] = {0};
        herr_t err = H5Gget_num_objs(topGroup.getId(), &nobj);
        for (int i = 0; i < nobj; ++i) {
            H5Gget_objname_by_idx(topGroup.getId(),
                                  (hsize_t)i,
                                  nameBuf,
                                  (size_t)BUFSIZE);
            int type = H5Gget_objtype_by_idx(topGroup.getId(), (hsize_t)i);
            if (type == H5G_GROUP) {
                groupNames.push_back(nameBuf);
            } else if (type == H5G_DATASET) {
                doDataset(cph5Top, topGroup, nameBuf);
            }
        }
        
        for (int i = 0; i < groupNames.size(); ++i) {
            CPH5Group *pNewGroup = new CPH5Group(&cph5Top, groupNames.at(i));
            H5::Group hg(topGroup.openGroup(groupNames.at(i)));
            cph5Top.registerExternalChild(pNewGroup);
            recurseGroups(*pNewGroup, hg);
        }
    }
    
    
public:
    
    static void dynamicGroup(CPH5Group &top, std::string filename) {
        
        // Open the file and the root group
        H5::H5File h5file(filename, H5F_ACC_RDONLY);
        H5::Group topGroup(h5file.openGroup("/"));
        
        recurseGroups(top, topGroup);
    }
    
};

template<> inline H5T_sign_t CPH5Dynamic::getH5Sign<int8_t>() { return H5T_SGN_2; }
template<> inline H5T_sign_t CPH5Dynamic::getH5Sign<int16_t>() { return H5T_SGN_2; }
template<> inline H5T_sign_t CPH5Dynamic::getH5Sign<int32_t>() { return H5T_SGN_2; }
template<> inline H5T_sign_t CPH5Dynamic::getH5Sign<int64_t>() { return H5T_SGN_2; }
template<> inline H5T_sign_t CPH5Dynamic::getH5Sign<uint8_t>() { return H5T_SGN_NONE; }
template<> inline H5T_sign_t CPH5Dynamic::getH5Sign<uint16_t>() { return H5T_SGN_NONE; }
template<> inline H5T_sign_t CPH5Dynamic::getH5Sign<uint32_t>() { return H5T_SGN_NONE; }
template<> inline H5T_sign_t CPH5Dynamic::getH5Sign<uint64_t>() { return H5T_SGN_NONE; }


template<>
inline bool CPH5Dynamic::memberAdderIf<float>(const H5::DataType &h5type, CPH5CompType *fill, std::string name) {
    if (h5type.getSize() == sizeof(float)) {
        auto *pAdd = new CPH5CompMember<float>(fill, name, h5type);
        fill->registerExternalMember(pAdd);
        return true;
    }
    return false;
}

template<>
inline bool CPH5Dynamic::memberAdderIf<double>(const H5::DataType &h5type, CPH5CompType *fill, std::string name) {
    if (h5type.getSize() == sizeof(double)) {
        auto *pAdd = new CPH5CompMember<double>(fill, name, h5type);
        fill->registerExternalMember(pAdd);
        return true;
    }
    return false;
}

template<>
inline bool CPH5Dynamic::arrayAddIf<float>(H5::DataType &h5type, CPH5CompType *fill, std::string name, int nElements) {
    if (h5type.getSize() == sizeof(float)) {
        auto *pAdd = new CPH5CompMemberArrayCommon<float, IS_NOT_DERIVED>(fill, name, h5type, nElements);
        fill->registerExternalMember(pAdd);
        return true;
    }
    return false;
}

template<>
inline bool CPH5Dynamic::arrayAddIf<double>(H5::DataType &h5type, CPH5CompType *fill, std::string name, int nElements) {
    if (h5type.getSize() == sizeof(double)) {
        auto *pAdd = new CPH5CompMemberArrayCommon<double, IS_NOT_DERIVED>(fill, name, h5type, nElements);
        fill->registerExternalMember(pAdd);
        return true;
    }
    return false;
}


void CPH5Dynamic::recurseComptype(const H5::CompType &h5type, CPH5CompType *fill) {
    // Recursively create the comptype here.
    int numMembers = h5type.getNmembers();
    std::vector<std::string> memNames;
    for (int i = 0; i < numMembers; ++i) {
        memNames.push_back(h5type.getMemberName(i));
    }

    for (int i = 0; i < numMembers; ++i) {
        H5T_class_t memClass = h5type.getMemberClass(i);
        if (memClass == H5T_COMPOUND) {
            auto *pCompToAdd = new CPH5CompMember<CPH5CompType>(fill,
                                                                memNames.at(i),
                                                                h5type.getMemberCompType(i));
            recurseComptype(h5type.getMemberCompType(i), dynamic_cast<CPH5CompType*>(pCompToAdd));
            fill->registerExternalMember(pCompToAdd);
        } else if (memClass == H5T_INTEGER) {
            bool success = false;
            success = success || memberAdderIf<int8_t>  (h5type.getMemberDataType(i), fill, memNames.at(i));
            success = success || memberAdderIf<int16_t> (h5type.getMemberDataType(i), fill, memNames.at(i));
            success = success || memberAdderIf<int32_t> (h5type.getMemberDataType(i), fill, memNames.at(i));
            success = success || memberAdderIf<int64_t> (h5type.getMemberDataType(i), fill, memNames.at(i));
            success = success || memberAdderIf<uint8_t> (h5type.getMemberDataType(i), fill, memNames.at(i));
            success = success || memberAdderIf<uint16_t>(h5type.getMemberDataType(i), fill, memNames.at(i));
            success = success || memberAdderIf<uint32_t>(h5type.getMemberDataType(i), fill, memNames.at(i));
            success = success || memberAdderIf<uint64_t>(h5type.getMemberDataType(i), fill, memNames.at(i));
            if (!success) {
                //TODO error here?
                throw "BAD FAIL";
            }
        } else if (memClass == H5T_FLOAT) {
            bool success = false;
            success = success || memberAdderIf<float>(h5type.getMemberDataType(i), fill, memNames.at(i));
            success = success || memberAdderIf<double>(h5type.getMemberDataType(i), fill, memNames.at(i));
            if (!success) {
                //TODO error here?
                throw "BAD FAIL";
            }
        } else if (memClass == H5T_ARRAY) {
            H5::ArrayType arrType(h5type.getMemberArrayType(i));
            int arrRank = arrType.getArrayNDims();
            // Only support 1-dimensional arrays right now
            if (arrRank == 1) {
                hsize_t nElements = 0;
                arrType.getArrayDims(&nElements);
                H5::DataType baseType = arrType.getSuper();
                H5T_class_t elemClass = baseType.getClass();
                if (elemClass == H5T_COMPOUND) {
                    auto *pCompArrToAdd = new CPH5CompMemberArrayCommon<CPH5CompType, IS_DERIVED>(fill, memNames.at(i), baseType, nElements);
                    for (int cid = 0; cid < nElements; ++cid) {
                        H5::CompType h5ct(baseType.getId());
                        recurseComptype(h5ct, dynamic_cast<CPH5CompType*>(pCompArrToAdd->getCompTypeObjAtBypass(cid)));
                        (pCompArrToAdd->getCompTypeObjAtBypass(cid))->setArrayParent(pCompArrToAdd);
                    }
                    fill->registerExternalMember(pCompArrToAdd);
                } else if (elemClass == H5T_INTEGER) {
                    bool success = false;
                    success = success || arrayAddIf<int8_t>  (baseType, fill, memNames.at(i), nElements);
                    success = success || arrayAddIf<int16_t> (baseType, fill, memNames.at(i), nElements);
                    success = success || arrayAddIf<int32_t> (baseType, fill, memNames.at(i), nElements);
                    success = success || arrayAddIf<int64_t> (baseType, fill, memNames.at(i), nElements);
                    success = success || arrayAddIf<uint8_t> (baseType, fill, memNames.at(i), nElements);
                    success = success || arrayAddIf<uint16_t>(baseType, fill, memNames.at(i), nElements);
                    success = success || arrayAddIf<uint32_t>(baseType, fill, memNames.at(i), nElements);
                    success = success || arrayAddIf<uint64_t>(baseType, fill, memNames.at(i), nElements);
                    if (!success) {
                        //TODO error unknown type?
                        throw "BAD UNKNOWN ARRAY TYPE";
                    }
                } else if (elemClass == H5T_FLOAT) {
                    bool success = false;
                    success = success || arrayAddIf<float> (baseType, fill, memNames.at(i), nElements);
                    success = success || arrayAddIf<double>(baseType, fill, memNames.at(i), nElements);
                    if (!success) {
                        //TODO error unknown type?
                        throw "BAD UNKNOWN ARRAY FLOAT TYPE";
                    }
                } else {
                    //TODO error here?
                }
            } else {
                //TODO error here?
            }
        } else {
            // TODO error unknown type?
            throw "BAD UNKNOWN TYPE";
        }
    }
}
#endif // CPH5DYNAMIC_H
