////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Ball Aerospace & Technologies Corp. All Rights Reserved.
//
// This program is free software; you can modify and/or redistribute it under
// the terms found in the accompanying LICENSE.txt file.
////////////////////////////////////////////////////////////////////////////////

#ifndef CPH5ATTRIBUTE_H
#define CPH5ATTRIBUTE_H

#include "cph5utilities.h"
#include "cph5comptype.h"
#include "H5Cpp.h"


/*!
 * \brief The CPH5AttributeBase class is a specializable base class for
 *        CPH5Attributes depending on whether the type is derived from
 *        CPH5CompType or not. This implementation is a non-inherited
 *        specialization, meaning that the type T is not compound.
 */
template<class T, const int>
class CPH5AttributeBase
{
    // NON-INHERITED SPECIALIZATION
public:
    
    /*!
     * \brief CPH5AttributeBase Constructor. Is passed an H5::DataType to use
     *        in the target file since this is a non-compound specialization.
     * \param type H5::DataType to use in the target HDF5 file.
     */
    CPH5AttributeBase(H5::DataType type)
        : mpAttribute(0),
          mDataType(type)
    {} // NOOP
    
    
    /*!
     * \brief operator = Assignment operator overload for convenience writing.
     * \param other T object reference to write to the target HDF5 file.
     */
    void operator=(const T &other) {
        if (mpAttribute == 0) {
            return;
        }
        mpAttribute->write(mDataType, &other);
    }
    
    
    /*!
     * \brief operator T Cast operator overload for convenience reading of
     *        attribute from the target HDF5 file.
     */
    operator T() {
        if (mpAttribute == 0) {
            return T();
        }
        T buf;
        mpAttribute->read(mDataType, &buf);
        return buf;
    }
    
    
    /*!
     * \brief read Standard read function for reading of the attribute from
     *        the target HDF5 file.
     * \param other T object reference to store read data into.
     */
    void read(T &other) {
        if (mpAttribute != 0) {
            mpAttribute->read(mDataType, &other);
        }
    }
    
    /*!
     * \brief write Standard write function for writing of the attribute into
     *        the target HDF5 file.
     * \param other T object reference to write to the target HDF5 file.
     */
    void write(const T &other) {
        if (mpAttribute != 0) {
            mpAttribute->write(mDataType, &other);
        }
    }
    
protected:
    
    H5::Attribute *mpAttribute;
    H5::DataType mDataType;
    
};




/*!
 * \brief The CPH5AttributeBase<T, _Tp2> class is a specializable base class
 *        for CPH5Attributes depending on whether the type is derived from
 *        CPH5CompType or not. This implementation is an inherited
 *        specialization, meaning that the type T is compound.
 */
template<class T>
class CPH5AttributeBase<T, IS_DERIVED>
{
    // INHERITED SPECIALIZATION
public:
    
    /*!
     * \brief CPH5AttributeBase Constructor. Does not get the H5::DataType as
     *        a parameter since the type is to be derived from T (which is
     *        compound).
     */
    CPH5AttributeBase()
        : mpAttribute(0),
          mDataType(T().getCompType())
    {} // NOOP
    
    /*!
     * \brief Reads the attribute data from the target file into the given T
     *        reference.
     * \param other T object to write data into
     */
    void read(T &other) {
        int size = other.getTotalMemorySize();
        char *buf = new char[size];
        char *ptr = buf;
        
        if (mpAttribute != 0) {
            mpAttribute->read(mDataType, buf);
        }
        other.latchAllAndMove(ptr);
        
        delete[] buf;
    }
    
    /*!
     * \brief Writes data from the given T object reference into the target
     *        file.
     * \param other T object containing data to write.
     */
    void write(const T &other) {
        int size = other.getTotalMemorySize();
        char *buf = new char[size];
        char *ptr = buf;
        
        other.copyAllAndMove(ptr);
        if (mpAttribute != 0) {
            mpAttribute->write(mDataType, buf);
        }
        
        delete[] buf;
    }
    
protected:
    
    H5::Attribute *mpAttribute;
    H5::DataType mDataType;
    
};







/*!
 * \brief The CPH5Attribute class creates an HDF5 Attribute with the specified
 *        type. The implementation of CPH5AttributeBase depends on whether the
 *        type T is compound. Attribute must belong to an AttributeHolder
 *        object, which in general are either Groups or Datasets.
 * 
 * Attributes are functionally similar to datasets with the key difference
 * that there is no slab-accessing of an attribute. In other words the entire
 * attribute must be read or written at once. 
 */
template<typename T>
class CPH5Attribute : public CPH5AttributeInterface,
        public CPH5AttributeBase<T, IsDerivedFrom<T, CPH5CompType>::Is>
{
	typedef CPH5AttributeBase<T, IsDerivedFrom<T, CPH5CompType>::Is> CPH5AttributeBaseSpec;
public:
    
    /*!
     * \brief CPH5Attribute Constructor to be used for non-compound
     *        construction. If a compound type is attempted to pass into this
     *        constructor a compile error will be generated.
     * \param parent Attribute holder implementation that is this attributes
     *        parent object (either a dataset or group).
     * \param name Name of this attribute visible in the target HDF5 file.
     * \param dataType H5::DataType of this attribute (normally an 
     *        H5::PredType).
     */
    CPH5Attribute(CPH5AttributeHolder *parent,
                  std::string name,
                  H5::DataType dataType)
        : CPH5AttributeInterface(name),
          CPH5AttributeBaseSpec(dataType),
          mpParent(parent)
    {
        if (mpParent)
            mpParent->registerAttribute(this);
        
        mDataSpace = H5::DataSpace(0, 0);
    }
    
    
    /*!
     * \brief CPH5Attribute Constructor to be used for compound type
     *        construction. If a non-compound type is attempted to pass into
     *        this constructor a compile error will be generated.
     * \param parent Attribute holder implementation that is this attributes
     *        parent object (either a dataset or group).
     * \param name Name of this attribute visible in the target HDF5 file.
     */
    CPH5Attribute(CPH5AttributeHolder *parent,
                  std::string name)
        : CPH5AttributeInterface(name),
          CPH5AttributeBaseSpec(),
          mpParent(parent)
    {
        if (mpParent)
            mpParent->registerAttribute(this);
        
        mDataSpace = H5::DataSpace(0, 0);
    }
        
    /*!
     * \brief Destructor. Calls closeR
     */
    virtual ~CPH5Attribute()
    {
        closeR();
    }
    
    /*!
     * \brief Recursive open function called from parent. Creates the
     *        H5::Attribute object, or opens it depending on the value of
     *        create. 
     * \param create Flag for whether to open or create the attribute.
     */
    void openR(bool create)
    {
        if (create)
            CPH5AttributeBaseSpec::mpAttribute = mpParent->createAttribute(mName,
                                                                           CPH5AttributeBaseSpec::mDataType,
                                                                           mDataSpace);
        else
            CPH5AttributeBaseSpec::mpAttribute = mpParent->openAttribute(mName);
    }
    
    
    /*!
     * \brief Recursive close function called from parent, usually during
     *        destruction. Deletes the attribute member if it exists.
     */
    void closeR()
    {
        if (CPH5AttributeBaseSpec::mpAttribute != 0) {
            CPH5AttributeBaseSpec::mpAttribute->close();
            delete CPH5AttributeBaseSpec::mpAttribute;
            CPH5AttributeBaseSpec::mpAttribute = 0;
        }
    }
    
    /*!
     * \brief operator = Assignment operator overload that passes the
     *        assignment to the CPH5AttributeBase implementation.
     * \param other T object reference to assign from.
     */
    void operator=(const T &other) {
        CPH5AttributeBaseSpec::operator=(other);
    }
    
    /*!
     * \brief operator = Overloaded assignment operator that copies the value
     *        other attribute into this one.
     * \param other
     */
    void operator=(CPH5Attribute<T> &other) {
        T val;
        other.read(val);
        write(val);
    }
    
    //TODO - for now, attributes do not support the tree concept
    
    //TODO document
    virtual CPH5LeafType getLeafType() const override {
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual bool getValIfLeaf(void * /*p*/) override {
        return false;
    }
    
    //TODO document
    virtual bool canIndexInto() const override {
        return false;
    }
    
    //TODO document
    virtual CPH5TreeNode *indexInto(int /*i*/) override {
        return 0;
    }
    
    //TODO document
    virtual int getIndexableSize() const override {
        return 0;
    }
    
    //TODO document
    virtual CPH5LeafType getElementType() const override {
        return LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual int getMemorySizeBelow() const override {
        return 0;
    }
    
    //TODO document
    virtual bool readAllBelow(void * /*p*/) override {
        return false;
    }
    
    virtual void *getMemoryLocation() const override {
        return 0;
    }
    
    
    //TODO document
    virtual std::vector<std::string> getChildrenNames() const override {
        return std::vector<std::string>();
    }
    
    //TODO document
    virtual CPH5TreeNode *getChildByName(std::string /*name*/) const override {
        return 0;
    }
    
private:
    
    CPH5AttributeHolder *mpParent;
    
    H5::DataSpace mDataSpace;
};

#endif // CPH5ATTRIBUTE_H
