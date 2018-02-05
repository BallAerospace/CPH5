////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Ball Aerospace & Technologies Corp. All Rights Reserved.
//
// This program is free software; you can modify and/or redistribute it under
// the terms found in the accompanying LICENSE.txt file.
////////////////////////////////////////////////////////////////////////////////

#ifndef CPH5UTILITIES_H
#define CPH5UTILITIES_H

#include "H5Cpp.h"
#include <vector>
#include <memory>

#define CPH_5_MAX_DIMS (32)


#ifdef _MSC_VER
#pragma warning(disable: 4355 4706 4115 4100 4201 4214 4054)
#endif /* _MSC_VER */

// Macros for the constructor initializer list, if applicable:
#define CPH5_CONSTRUCT(a) a(this, #a)
#define CPH5_CONSTRUCT_FLOAT(a) a(this, #a, H5::PredType::NATIVE_FLOAT)

namespace CPH5Swappers {
    
    inline static void swap_in_place(uint8_t *y)
    {
        // Do nothing
    }
    
    inline static void swap_in_place(int8_t *y)
    {
        // Do nothing
    }
    
    inline static void swap_in_place(char *y)
    {
        // Do nothing
    }

    inline static void swap_in_place(uint16_t *y)
    {
        uint16_t x = *y;
        x = (x>>8) | 
            (x<<8);
        *y = x;
    }
    
    inline static void swap_in_place(uint32_t *y)
    {
        uint32_t x = *y;
        x = (x>>24) | 
            ((x<<8) & 0x00FF0000) |
            ((x>>8) & 0x0000FF00) |
            (x<<24);
        *y = x;
    }
    
    // __int64 for MSVC, "long long" for gcc
    inline static void swap_in_place(uint64_t *y)
    {
        uint64_t x = *y;
        x = (x>>56) | 
            ((x<<40) & 0x00FF000000000000) |
            ((x<<24) & 0x0000FF0000000000) |
            ((x<<8)  & 0x000000FF00000000) |
            ((x>>8)  & 0x00000000FF000000) |
            ((x>>24) & 0x0000000000FF0000) |
            ((x>>40) & 0x000000000000FF00) |
            (x<<56);
        *y = x;
    }
    
    inline static void swap_in_place(int16_t *x) {
        swap_in_place(reinterpret_cast<uint16_t*>(x));
    }
    
    inline static void swap_in_place(int32_t *x) {
        swap_in_place(reinterpret_cast<uint32_t*>(x));
    }
    
    inline static void swap_in_place(float *x) {
        swap_in_place(reinterpret_cast<uint32_t*>(x));
    }
    
    inline static void swap_in_place(int64_t *x) {
        swap_in_place(reinterpret_cast<uint64_t*>(x));
    }
    
    inline static void swap_in_place(double *x) {
        swap_in_place(reinterpret_cast<uint64_t*>(x));
    }
    
    inline static void SwapInPlace32(void *p) {
        swap_in_place(reinterpret_cast<uint32_t*>(p));
    }
    
    inline static void SwapInPlace16(void *p) {
        swap_in_place(reinterpret_cast<uint16_t*>(p));
    }
    
    inline static void SwapInPlace64(void *p) {
        swap_in_place(reinterpret_cast<uint64_t*>(p));
    }
    
}




/*!
 * \brief The CPH5IOFacility class is a convenience object
 *        for maintaining hyperslab selections through layers
 *        of a multidimensional array or compound datatype.
 */
class CPH5IOFacility
{
public:
    
    /*!
     * \brief CPH5IOFacility Default constructor.
     */
    CPH5IOFacility()
        : mpDataSet(0),
          numDims(-1)
    {
        
    }
    
    
    /*!
     * \brief Initializes the IOFacility with the necessary parameters to begin
     *        hyperslab selection.
     * \param pDataSet Pointer to H5::DataSet object to use to read and write.
     * \param type H5::DataType that is assigned to the dataset. This is not
     *        necessarily used as other datatypes can be given in the read and
     *        write functions. This one serves as a default.
     * \param nDims The number of dimensions of the dataset.
     * \param maxDims The maximums of each dimension of the dataset.
     *        H5S_UNLIMITED is used to identify an unlimited dimension.
     */
    void init(H5::DataSet *pDataSet,
              H5::DataType type,
              int nDims,
              hsize_t *maxDims) {
        mpDataSet = pDataSet;
        mType = type;
        numDims = nDims;
        mMaxDims.clear();
        mIndices.clear();
        for (int i = 0; i < nDims; ++i) {
            mMaxDims.push_back(maxDims[i]);
        }
    }
    
    
    /*!
     * \brief Used when an index is selected for one of the dataset dimensions.
     *        It is stored into the IOFacility and used later upon a call to
     *        read or write.
     * \param ind Index of dimension.
     */
    void addIndex(int ind) {
        if (numDims == -1) {
            // BIG PROBLEM, UNINITIALIZED
            return;
        }
        mIndices.push_back(ind);
        if (mIndices.size() > numDims) {
            // BIG PROBLEM, TOO MANY INDICES
        }
    }
    
    /*!
     * \brief Write data from the given buffer to the target HDF5 file through
     *        the H5::DataSet object given to this object from init(). It is
     *        assumed that there is enough data in the buffer to source all
     *        the data needed below this point in the data tree.
     * \param src Buffer to source data from.
     */
    void write(const void *src) {
        if (mpDataSet == 0) {
            return;
        }
        setupSpaces();
        mpDataSet->write(src, mType, mMemspace, mFilespace);
    }
    
    
    /*!
     * \brief Overload of the other write function that allows the user to 
     *        specify a type different from the one given to this IOFacility
     *        object during init(). Commonly used by compound types with a
     *        datatype containing a subset of fields.
     * \param src Buffer to source data from.
     * \param type Datatype to use during the write.
     */
    void write(const void *src, H5::DataType type) {
        if (mpDataSet == 0) {
            return;
        }
        setupSpaces();
        mpDataSet->write(src, type, mMemspace, mFilespace);
    }
    
    
    /*!
     * \brief Special version of the basic write function that allows for
     *        writing to the dataset starting at a certain offset index.
     * \param src Buffer to source data from.
     */
    void writeWithOffset(int offset, const void *src) {
        if (mpDataSet == 0) {
            return;
        }
        setupSpacesOffset(offset);
        mpDataSet->write(src, mType, mMemspace, mFilespace);
    }
    
    
    /*!
     * \brief Read data from the target HDF5 file through the given H5::DataSet
     *        object into the buffer. It is assumed that the buffer is large
     *        enough to store all the data existing below this point in the
     *        data tree.
     * \param dst Buffer to store data into.
     */
    void read(void *dst) {
        if (mpDataSet == 0) {
            return;
        }
        setupSpaces();
        mpDataSet->read(dst, mType, mMemspace, mFilespace);
    }
    
    
    /*!
     * \brief Overload of the other read function that allows the user to
     *        specify a type different from the one given to this IOFacility
     *        object during init(). Commonly used by compound types with a
     *        datatype containing a subset of fields.
     * \param dst Buffer to store data into.
     * \param type Datatype to use during the read.
     */
    void read(void *dst, H5::DataType type) {
        if (mpDataSet == 0) {
            return;
        }
        setupSpaces();
        mpDataSet->read(dst, type, mMemspace, mFilespace);
        
    }
    
    
    /*!
     * \brief Calculates the total of selected elements that are currently
     *        selected in the dataset.
     * \return Number of selected elements.
     */
    hsize_t getNumLowerElements() {
        setupSpaces();
        return mFilespace.getSelectNpoints();
    }
    
    
    /*!
     * \brief Gets the total size of all the selected elements in bytes.
     * \return The size of all the selected elements in bytes.
     */
    hsize_t getSizeLowerElements() {
        setupSpaces();
        return mMemspace.getSelectNpoints()*mType.getSize();
    }
    
    
    /*!
     * \brief getIndices Returns the current list of indices
     * \return A copy of the list of indices
     */
    std::vector<int> getIndices() const {
        return mIndices;
    }
    
    /*!
     * \brief setIndices Sets the currently selected indices
     * \param indices
     */
    void setIndices(std::vector<int> &indices) {
        mIndices = indices;
    }
    
    
    
private:
    
    /*!
     * \brief This function is used to set up the dataspaces necessary for a
     *        hyperslab selection with the indexes added to this IOFacility
     *        with addIndex(). This should be done before any reads or writes
     *        to the H5::DataSet object given during init().
     */
    void setupSpaces() {
        if (numDims == -1) {
            // BIG PROBLEM
            return;
        }
        hsize_t offsets[CPH_5_MAX_DIMS];
        memset(offsets, 0, CPH_5_MAX_DIMS*4);
        hsize_t extents[CPH_5_MAX_DIMS];
        for (int i = 0; i < mIndices.size(); ++i) {
            offsets[i] = mIndices[i];
        }
        for (int i = 0; i < numDims; ++i) {
            if (i < mIndices.size()) {
                extents[i] = 1;
            } else {
                extents[i] = mMaxDims[i];
            }
        }
        
        if (mpDataSet != 0) {
            mFilespace = mpDataSet->getSpace();
            mMemspace = H5::DataSpace();
            if (numDims != 0) {
                mFilespace.selectHyperslab(H5S_SELECT_SET, extents, offsets);
                mMemspace = H5::DataSpace(numDims, extents, NULL);
            }
        }
    }
    
    
    /*!
     * \brief This function is used to set up the dataspaces necessary for a
     *        hyperslab selection with the indexes added to this IOFacility
     *        with addIndex(), as well as the offset passed in. The offset
     *        parameter is where the writing should begin.
     */
    void setupSpacesOffset(int offset) {
        if (numDims == -1) {
            // BIG PROBLEM
            return;
        }
        hsize_t offsets[CPH_5_MAX_DIMS];
        memset(offsets, 0, CPH_5_MAX_DIMS*4);
        hsize_t extents[CPH_5_MAX_DIMS];
        for (int i = 0; i < mIndices.size(); ++i) {
            offsets[i] = mIndices[i];
        }
        offsets[mIndices.size()] = offset;
        for (int i = 0; i < numDims; ++i) {
            if (i < mIndices.size()) {
                extents[i] = 1;
            } else if (i == mIndices.size()) {
                extents[i] = mMaxDims[i] - offset;
            } else {
                extents[i] = mMaxDims[i];
            }
        }
        
        if (mpDataSet != 0) {
            mFilespace = mpDataSet->getSpace();
            mMemspace = H5::DataSpace();
            if (numDims != 0) {
                mFilespace.selectHyperslab(H5S_SELECT_SET, extents, offsets);
                mMemspace = H5::DataSpace(numDims, extents, NULL);
            }
        }
    }
    
    H5::DataSet *mpDataSet;
    H5::DataType mType;
    
    int numDims;
    std::vector<int> mMaxDims;
    std::vector<int> mIndices;
    
    H5::DataSpace mMemspace;
    H5::DataSpace mFilespace;
};



/*!
 * \brief The IsDerivedFrom class is a utility class used to
 *        help solve the SFINAE problem. hashtag Stolen
 * 
 * This class cannot be implemented, it can only be used as a
 * compile-time hook for determining whether or not a class
 * given as a template parameter is derived from another class.
 * See the CPH5DataSet class for usage.
 */
template<typename D, typename B>
class IsDerivedFrom
{
    /*!
     * \cond
     */
    class No { };
    class Yes { No no[3]; }; 
    
    static Yes Test( B* ); // not defined
    static No Test( ... ); // not defined 
    
    static void Constraints(D* p) { B* pb = p; pb = p; } 
    
public:
    enum { Is = sizeof(Test(static_cast<D*>(0))) == sizeof(Yes) }; 
    
    IsDerivedFrom() { void(*p)(D*) = Constraints; }
    /*!
     * \endcond
     */
};
class Base { };
class Derived : public Base { };
class NotDerived { };
enum {
    IS_DERIVED = IsDerivedFrom<Derived, Base>::Is,
    IS_NOT_DERIVED = IsDerivedFrom<NotDerived, Base>::Is
};


/*!
 * \brief The CPH5TreeNode class is a base interface class for all
 *        objects in a CPH5 structure that allows traversing the
 *        tree and retrieving whether the node is a leaf (terminal)
 *        or not, and if so retrieving its value.
 */
class CPH5TreeNode {
public:
    
    enum CPH5LeafType {
        LT_IS_NOT_LEAF = 0,
        LT_UINT8,
        LT_UINT16,
        LT_UINT32,
        LT_UINT64,
        LT_INT8,
        LT_INT16,
        LT_INT32,
        LT_INT64,
        LT_FLOAT,
        LT_DOUBLE,
        LT_STRING
    };
    
    // Utility structs for determing leaf based on type
    template<typename T>
    struct IsLeaf {
        enum { Get = LT_IS_NOT_LEAF };
    };

    
    virtual CPH5LeafType getLeafType() const = 0;
    virtual bool getValIfLeaf(void *p) = 0;
    
    virtual bool canIndexInto() const = 0;
    virtual CPH5TreeNode *indexInto(int i) = 0;
    virtual int getIndexableSize() const = 0;
    
    virtual CPH5LeafType getElementType() const = 0;
    virtual int getMemorySizeBelow() const = 0;
    virtual bool readAllBelow(void *p) = 0;
    virtual void *getMemoryLocation() const = 0;
    
    virtual std::vector<std::string> getChildrenNames() const = 0;
    virtual CPH5TreeNode *getChildByName(std::string name) const = 0;
    
    // Currently known constraint of walking a non-scalar dataset with
    // the tree nodes: indexing into the tree will cause it to overwrite
    // any prior indexes. I.e. you must always start from the top of the 
    // dataset when changing any indices.
    //TODO fix this somehow
};


template<>
struct CPH5TreeNode::IsLeaf<uint8_t> {
    enum { Get = LT_UINT8 };
};
template<>
struct CPH5TreeNode::IsLeaf<uint16_t> {
    enum { Get = LT_UINT16 };
};
template<>
struct CPH5TreeNode::IsLeaf<uint32_t> {
    enum { Get = LT_UINT32 };
};
template<>
struct CPH5TreeNode::IsLeaf<uint64_t> {
    enum { Get = LT_UINT64 };
};
template<>
struct CPH5TreeNode::IsLeaf<int8_t> {
    enum { Get = LT_INT8 };
};
template<>
struct CPH5TreeNode::IsLeaf<int16_t> {
    enum { Get = LT_INT16 };
};
template<>
struct CPH5TreeNode::IsLeaf<int32_t> {
    enum { Get = LT_INT32 };
};
template<>
struct CPH5TreeNode::IsLeaf<int64_t> {
    enum { Get = LT_INT64 };
};
template<>
struct CPH5TreeNode::IsLeaf<float> {
    enum { Get = LT_FLOAT };
};
template<>
struct CPH5TreeNode::IsLeaf<double> {
    enum { Get = LT_DOUBLE };
};
template<>
struct CPH5TreeNode::IsLeaf<std::string> {
    enum { Get = LT_STRING };
};

/*!
 * \brief The CPH5GroupMember class is a base interface class
 *        that is inherited in order to identify an object
 *        as capable of belonging to a HDF5 Group.
 * 
 * Classes derived from this are referenced by pointer by the
 * group container that they belong to. Such classes must also
 * accept a CPH5Group parent pointer and call register in their
 * constructor. The reason that this class does not handle that
 * functionality is that groups can belong to other groups, as
 * well as the fact that certain things like Attributes can
 * be parented by things other than groups.
 */
class CPH5GroupMember : public CPH5TreeNode
{
public:
    
    /*!
     * \brief CPH5GroupMember constructor that accepts the name of the HDF5
     *        element, since all group members must be locatable by name.
     * \param name Name of element.
     */
    CPH5GroupMember(std::string name)
        : mName(name)
    {
        //NOOP
    }
    
    virtual ~CPH5GroupMember() {}
    
    /*!
     * \brief Recursive open function interface.
     * \param create Flag for whether or not to create the item or
     *        to open it.
     */
    virtual void openR(bool create) = 0;
    
    /*!
     * \brief closeR Recursive close function. This cannot be an interface
     *        due to the use of virtual destructors.
     */
    virtual void closeR() {}
    
    //TODO document
    virtual int numChildren() const {
       return 0;
    }
    
    virtual CPH5GroupMember *childAt(int) const {
       return 0;
    }
    
    //TODO document
    virtual std::string getName() const {
       return mName;
    }
    
protected:
    
    std::string mName;
};



/*!
 * \brief The CPH5AttributeBase class is basically a pass-through of the 
 *        CPH5GroupMember class that is used to differentiate Attributes
 *        from the other group members since Attributes can be attached
 *        to Datasets as well as groups.
 */
class CPH5AttributeInterface : public CPH5GroupMember
{
public:
    
    /*!
     * \brief CPH5AttributeInterface pass through constructor for the element
     *        name.
     * \param name Name of HDF5 element.
     */
    CPH5AttributeInterface(std::string name)
        : CPH5GroupMember(name)
    {} // NOOP
    
    
    
};







/*!
 * \brief The CPH5AttributeHolder class is a base interface 
 *        class that defines the functionality of an object
 *        to have HDF5 Attributes.
 * 
 * Classes derived from this must instantiate the abstract
 * functions and then maintain a list of CPH5Attribute
 * children. Group classes lump Attribute children into the
 * same container as DataSet children, while DataSets maintain
 * a list specific for Attributes.
 */
class CPH5AttributeHolder
{
public:
    /*!
     * \brief createAttribute Interface for implementation specific parameters
     *        needed to create an H5::Attribute
     * \param name Name of attribute.
     * \param dataType H5::DataType of attribute.
     * \param space H5::DataSpace representing the filespace of the attribute.
     * \return Pointer to newly created attribute, or 0 if failure.
     */
    virtual H5::Attribute *createAttribute(std::string name,
                                           H5::DataType dataType,
                                           H5::DataSpace space) = 0;
    
    /*!
     * \brief openAttribute Interface for implementation specific parameters
     *        needed to open an H5::Attribute
     * \param name Name of attribute to open.
     * \return Pointer to newly created attribute, or 0 if failure.
     */
    virtual H5::Attribute *openAttribute(std::string name) = 0;
    
    /*!
     * \brief registerAttribute Interface for implementation specific 
     *        registering that a CPH5Attribute is a child of this object.
     * \param child Pointer to CPH5Attribute to register.
     */
    virtual void registerAttribute(CPH5AttributeInterface *child) = 0;
    
    /*!
     * \brief unregisterAttribute Interface for implementation specific
     *        unregistering of a CPH5Attribute from this object.
     * \param child Pointer to CPH5Attribute to unregister.
     */
    virtual void unregisterAttribute(const CPH5AttributeInterface *child) = 0;
};







/*!
 * \brief The CPH5TypeProxy class is a pass-through class for
 *        types commonly used in the HDF5 files created with the
 *        CPH5 Library. It can be used in any place where an 
 *        H5::DataType is expected.
 * 
 * Currently supported types for this class are:
 * <ul>
 *    <li>double   - H5::PredType::NATIVE_DOUBLE </li>
 *    <li>float    - H5::PredType::NATIVE_FLOAT  </li>
 *    <li>uint8_t  - H5::PredType::NATIVE_UINT8  </li>
 *    <li>uint16_t - H5::PredType::NATIVE_UINT16 </li>
 *    <li>uint32_t - H5::PredType::NATIVE_UINT32 </li>
 *    <li>uint64_t - H5::PredType::NATIVE_UINT64 </li>
 *    <li>int8_t   - H5::PredType::NATIVE_INT8   </li>
 *    <li>int16_t  - H5::PredType::NATIVE_INT16  </li>
 *    <li>int32_t  - H5::PredType::NATIVE_INT32  </li>
 *    <li>int64_t  - H5::PredType::NATIVE_INT64  </li>
 *    <li>char     - H5::PredType::NATIVE_CHAR   </li>
 * </ul>
 * Instantiating this class without one of these types will result
 * in a compile error.
 */
template<typename T>
class CPH5TypeProxy {
public:
    operator H5::DataType(); // Undefined on purpose
};

/*!
 * \cond
 */
template<>
class CPH5TypeProxy<double> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_DOUBLE;
    }
};

template<>
class CPH5TypeProxy<float> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_FLOAT;
    }
};

template<>
class CPH5TypeProxy<uint8_t> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_UINT8;
    }
};

template<>
class CPH5TypeProxy<uint16_t> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_UINT16;
    }
};

template<>
class CPH5TypeProxy<uint32_t> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_UINT32;
    }
};

template<>
class CPH5TypeProxy<uint64_t> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_UINT64;
    }
};

template<>
class CPH5TypeProxy<int8_t> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_INT8;
    }
};

template<>
class CPH5TypeProxy<int16_t> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_INT16;
    }
};

template<>
class CPH5TypeProxy<int32_t> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_INT32;
    }
};

template<>
class CPH5TypeProxy<int64_t> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_INT64;
    }
};

template<>
class CPH5TypeProxy<char> {
public:
    operator H5::DataType() {
        return H5::PredType::NATIVE_CHAR;
    }
};



#include <iostream>

class CPH5Utilities {
public:
    //Temporary functions
    static void printTypeInfo(const H5::DataType &type, std::string indent = "") {
        std::cout << indent << "Type Size: " << type.getSize() << std::endl;
        std::cout << indent << "Class: ";
        switch (type.getClass()) {
            case (H5T_NO_CLASS):
                std::cout << indent << "H5T_NO_CLASS" << std::endl;
                break;
            case (H5T_INTEGER):
                std::cout << indent << "H5T_INTEGER" << std::endl;
                break;
            case (H5T_FLOAT):
                std::cout << indent << "H5T_FLOAT" << std::endl;
                break;
            case (H5T_COMPOUND): {
                std::cout << indent << "H5T_COMPOUND" << std::endl;
                H5::CompType ct(type.getId());
                std::cout << indent << "CompType {\n";
                for (int i = 0; i < ct.getNmembers(); ++i) {
                    std::cout << indent << "   " << "Member 0 {\n";
                    printTypeInfo(ct.getMemberDataType(i), indent+"   ");
                    std::cout << indent << "   " << "}\n";
                }
                std::cout << indent << "}\n";
                break; }
            case (H5T_ARRAY): {
                std::cout << indent << "H5T_ARRAY" << std::endl;
                H5::ArrayType at(type.getId());
                hsize_t dim = 0;
                at.getArrayDims(&dim);
                std::cout << indent << "Num elements: " << dim << std::endl;
                std::cout << indent << "BaseType {\n";
                printTypeInfo(type.getSuper(), indent+"   ");
                std::cout << indent << "}\n";
                break; }
            default: std::cout << indent << "UNKNOWN\n";
        }
        
    }
};

/*!
 * \endcond
 */




#endif // CPH5UTILITIES_H
