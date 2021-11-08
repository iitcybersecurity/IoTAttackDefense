//
// Generated file, do not edit! Created by opp_msgc 4.2 from applications/myapplication/MyMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "MyMessage_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("MessageType");
    if (!e) enums.getInstance()->add(e = new cEnum("MessageType"));
    e->insert(MYMSG_GET_OPENSHS_DATA, "MYMSG_GET_OPENSHS_DATA");
    e->insert(MYMSG_GET_OPENSHS_RESPONSE, "MYMSG_GET_OPENSHS_RESPONSE");
    e->insert(MYMSG_PUT_OPENSHS_DATA, "MYMSG_PUT_OPENSHS_DATA");
    e->insert(MYMSG_PUT_CLASSIFICATION_DATA, "MYMSG_PUT_CLASSIFICATION_DATA");
    e->insert(MYMSG_GET_CLASSIFICATION_DATA, "MYMSG_GET_CLASSIFICATION_DATA");
    e->insert(MYMSG_PUT_TEMPERATURE, "MYMSG_PUT_TEMPERATURE");
    e->insert(MYMSG_PUT_MOTION, "MYMSG_PUT_MOTION");
    e->insert(MYMSG_GET_TEMPERATURE, "MYMSG_GET_TEMPERATURE");
    e->insert(MYMSG_GET_MOTION, "MYMSG_GET_MOTION");
    e->insert(MYMSG_GET_TEMPERATURE_RESPONSE, "MYMSG_GET_TEMPERATURE_RESPONSE");
    e->insert(MYMSG_GET_MOTION_RESPONSE, "MYMSG_GET_MOTION_RESPONSE");
    e->insert(MYMSG_GET_CLASSIFICATION_DATA_RESPONSE, "MYMSG_GET_CLASSIFICATION_DATA_RESPONSE");
    e->insert(MYMSG_PUT_REPUTATION, "MYMSG_PUT_REPUTATION");
    e->insert(MYMSG_GET_REPUTATION, "MYMSG_GET_REPUTATION");
    e->insert(MYMSG_GET_REPUTATION_RESPONSE, "MYMSG_GET_REPUTATION_RESPONSE");
    e->insert(MYMSG_PUT_VOTE, "MYMSG_PUT_VOTE");
    e->insert(MYMSG_GET_VOTE, "MYMSG_GET_VOTE");
    e->insert(MYMSG_GET_VOTE_RESPONSE, "MYMSG_GET_VOTE_RESPONSE");
);

Register_Class(MyMessage);

MyMessage::MyMessage(const char *name, int kind) : cPacket(name,kind)
{
    this->type_var = 0;
    this->requesterNodeId_var = 0;
    this->targetNodeId_var = 0;
    this->ownerNodeId_var = 0;
    this->detectedValue_var = 0;
    this->temperatureValue_var = 0;
    this->motionValue_var = 0;
    for (unsigned int i=0; i<10; i++)
        this->classificationData_var[i] = 0;
    for (unsigned int i=0; i<2; i++)
        this->reputationValue_var[i] = 0;
    this->voteValue_var = 0;
    this->isInvalidData_var = 0;
    this->TimeSlot_var = 0;
}

MyMessage::MyMessage(const MyMessage& other) : cPacket(other)
{
    copy(other);
}

MyMessage::~MyMessage()
{
}

MyMessage& MyMessage::operator=(const MyMessage& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void MyMessage::copy(const MyMessage& other)
{
    this->type_var = other.type_var;
    this->senderAddress_var = other.senderAddress_var;
    this->key_var = other.key_var;
    this->requesterNodeId_var = other.requesterNodeId_var;
    this->targetNodeId_var = other.targetNodeId_var;
    this->ownerNodeId_var = other.ownerNodeId_var;
    this->detectedValue_var = other.detectedValue_var;
    this->temperatureValue_var = other.temperatureValue_var;
    this->motionValue_var = other.motionValue_var;
    for (unsigned int i=0; i<10; i++)
        this->classificationData_var[i] = other.classificationData_var[i];
    for (unsigned int i=0; i<2; i++)
        this->reputationValue_var[i] = other.reputationValue_var[i];
    this->voteValue_var = other.voteValue_var;
    this->isInvalidData_var = other.isInvalidData_var;
    this->TimeSlot_var = other.TimeSlot_var;
}

void MyMessage::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->type_var);
    doPacking(b,this->senderAddress_var);
    doPacking(b,this->key_var);
    doPacking(b,this->requesterNodeId_var);
    doPacking(b,this->targetNodeId_var);
    doPacking(b,this->ownerNodeId_var);
    doPacking(b,this->detectedValue_var);
    doPacking(b,this->temperatureValue_var);
    doPacking(b,this->motionValue_var);
    doPacking(b,this->classificationData_var,10);
    doPacking(b,this->reputationValue_var,2);
    doPacking(b,this->voteValue_var);
    doPacking(b,this->isInvalidData_var);
    doPacking(b,this->TimeSlot_var);
}

void MyMessage::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->senderAddress_var);
    doUnpacking(b,this->key_var);
    doUnpacking(b,this->requesterNodeId_var);
    doUnpacking(b,this->targetNodeId_var);
    doUnpacking(b,this->ownerNodeId_var);
    doUnpacking(b,this->detectedValue_var);
    doUnpacking(b,this->temperatureValue_var);
    doUnpacking(b,this->motionValue_var);
    doUnpacking(b,this->classificationData_var,10);
    doUnpacking(b,this->reputationValue_var,2);
    doUnpacking(b,this->voteValue_var);
    doUnpacking(b,this->isInvalidData_var);
    doUnpacking(b,this->TimeSlot_var);
}

int MyMessage::getType() const
{
    return type_var;
}

void MyMessage::setType(int type)
{
    this->type_var = type;
}

TransportAddress& MyMessage::getSenderAddress()
{
    return senderAddress_var;
}

void MyMessage::setSenderAddress(const TransportAddress& senderAddress)
{
    this->senderAddress_var = senderAddress;
}

OverlayKey& MyMessage::getKey()
{
    return key_var;
}

void MyMessage::setKey(const OverlayKey& key)
{
    this->key_var = key;
}

int MyMessage::getRequesterNodeId() const
{
    return requesterNodeId_var;
}

void MyMessage::setRequesterNodeId(int requesterNodeId)
{
    this->requesterNodeId_var = requesterNodeId;
}

int MyMessage::getTargetNodeId() const
{
    return targetNodeId_var;
}

void MyMessage::setTargetNodeId(int targetNodeId)
{
    this->targetNodeId_var = targetNodeId;
}

int MyMessage::getOwnerNodeId() const
{
    return ownerNodeId_var;
}

void MyMessage::setOwnerNodeId(int ownerNodeId)
{
    this->ownerNodeId_var = ownerNodeId;
}

const char * MyMessage::getDetectedValue() const
{
    return detectedValue_var.c_str();
}

void MyMessage::setDetectedValue(const char * detectedValue)
{
    this->detectedValue_var = detectedValue;
}

const char * MyMessage::getTemperatureValue() const
{
    return temperatureValue_var.c_str();
}

void MyMessage::setTemperatureValue(const char * temperatureValue)
{
    this->temperatureValue_var = temperatureValue;
}

const char * MyMessage::getMotionValue() const
{
    return motionValue_var.c_str();
}

void MyMessage::setMotionValue(const char * motionValue)
{
    this->motionValue_var = motionValue;
}

unsigned int MyMessage::getClassificationDataArraySize() const
{
    return 10;
}

const char * MyMessage::getClassificationData(unsigned int k) const
{
    if (k>=10) throw cRuntimeError("Array of size 10 indexed by %lu", (unsigned long)k);
    return classificationData_var[k].c_str();
}

void MyMessage::setClassificationData(unsigned int k, const char * classificationData)
{
    if (k>=10) throw cRuntimeError("Array of size 10 indexed by %lu", (unsigned long)k);
    this->classificationData_var[k] = classificationData;
}

unsigned int MyMessage::getReputationValueArraySize() const
{
    return 2;
}

double MyMessage::getReputationValue(unsigned int k) const
{
    if (k>=2) throw cRuntimeError("Array of size 2 indexed by %lu", (unsigned long)k);
    return reputationValue_var[k];
}

void MyMessage::setReputationValue(unsigned int k, double reputationValue)
{
    if (k>=2) throw cRuntimeError("Array of size 2 indexed by %lu", (unsigned long)k);
    this->reputationValue_var[k] = reputationValue;
}

double MyMessage::getVoteValue() const
{
    return voteValue_var;
}

void MyMessage::setVoteValue(double voteValue)
{
    this->voteValue_var = voteValue;
}

bool MyMessage::getIsInvalidData() const
{
    return isInvalidData_var;
}

void MyMessage::setIsInvalidData(bool isInvalidData)
{
    this->isInvalidData_var = isInvalidData;
}

int MyMessage::getTimeSlot() const
{
    return TimeSlot_var;
}

void MyMessage::setTimeSlot(int TimeSlot)
{
    this->TimeSlot_var = TimeSlot;
}

class MyMessageDescriptor : public cClassDescriptor
{
  public:
    MyMessageDescriptor();
    virtual ~MyMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(MyMessageDescriptor);

MyMessageDescriptor::MyMessageDescriptor() : cClassDescriptor("MyMessage", "cPacket")
{
}

MyMessageDescriptor::~MyMessageDescriptor()
{
}

bool MyMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<MyMessage *>(obj)!=NULL;
}

const char *MyMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int MyMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 14+basedesc->getFieldCount(object) : 14;
}

unsigned int MyMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISARRAY | FD_ISEDITABLE,
        FD_ISARRAY | FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<14) ? fieldTypeFlags[field] : 0;
}

const char *MyMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "type",
        "senderAddress",
        "key",
        "requesterNodeId",
        "targetNodeId",
        "ownerNodeId",
        "detectedValue",
        "temperatureValue",
        "motionValue",
        "classificationData",
        "reputationValue",
        "voteValue",
        "isInvalidData",
        "TimeSlot",
    };
    return (field>=0 && field<14) ? fieldNames[field] : NULL;
}

int MyMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderAddress")==0) return base+1;
    if (fieldName[0]=='k' && strcmp(fieldName, "key")==0) return base+2;
    if (fieldName[0]=='r' && strcmp(fieldName, "requesterNodeId")==0) return base+3;
    if (fieldName[0]=='t' && strcmp(fieldName, "targetNodeId")==0) return base+4;
    if (fieldName[0]=='o' && strcmp(fieldName, "ownerNodeId")==0) return base+5;
    if (fieldName[0]=='d' && strcmp(fieldName, "detectedValue")==0) return base+6;
    if (fieldName[0]=='t' && strcmp(fieldName, "temperatureValue")==0) return base+7;
    if (fieldName[0]=='m' && strcmp(fieldName, "motionValue")==0) return base+8;
    if (fieldName[0]=='c' && strcmp(fieldName, "classificationData")==0) return base+9;
    if (fieldName[0]=='r' && strcmp(fieldName, "reputationValue")==0) return base+10;
    if (fieldName[0]=='v' && strcmp(fieldName, "voteValue")==0) return base+11;
    if (fieldName[0]=='i' && strcmp(fieldName, "isInvalidData")==0) return base+12;
    if (fieldName[0]=='T' && strcmp(fieldName, "TimeSlot")==0) return base+13;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *MyMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "TransportAddress",
        "OverlayKey",
        "int",
        "int",
        "int",
        "string",
        "string",
        "string",
        "string",
        "double",
        "double",
        "bool",
        "int",
    };
    return (field>=0 && field<14) ? fieldTypeStrings[field] : NULL;
}

const char *MyMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "MessageType";
            return NULL;
        default: return NULL;
    }
}

int MyMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    MyMessage *pp = (MyMessage *)object; (void)pp;
    switch (field) {
        case 9: return 10;
        case 10: return 2;
        default: return 0;
    }
}

std::string MyMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    MyMessage *pp = (MyMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getType());
        case 1: {std::stringstream out; out << pp->getSenderAddress(); return out.str();}
        case 2: {std::stringstream out; out << pp->getKey(); return out.str();}
        case 3: return long2string(pp->getRequesterNodeId());
        case 4: return long2string(pp->getTargetNodeId());
        case 5: return long2string(pp->getOwnerNodeId());
        case 6: return oppstring2string(pp->getDetectedValue());
        case 7: return oppstring2string(pp->getTemperatureValue());
        case 8: return oppstring2string(pp->getMotionValue());
        case 9: return oppstring2string(pp->getClassificationData(i));
        case 10: return double2string(pp->getReputationValue(i));
        case 11: return double2string(pp->getVoteValue());
        case 12: return bool2string(pp->getIsInvalidData());
        case 13: return long2string(pp->getTimeSlot());
        default: return "";
    }
}

bool MyMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    MyMessage *pp = (MyMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setType(string2long(value)); return true;
        case 3: pp->setRequesterNodeId(string2long(value)); return true;
        case 4: pp->setTargetNodeId(string2long(value)); return true;
        case 5: pp->setOwnerNodeId(string2long(value)); return true;
        case 6: pp->setDetectedValue((value)); return true;
        case 7: pp->setTemperatureValue((value)); return true;
        case 8: pp->setMotionValue((value)); return true;
        case 9: pp->setClassificationData(i,(value)); return true;
        case 10: pp->setReputationValue(i,string2double(value)); return true;
        case 11: pp->setVoteValue(string2double(value)); return true;
        case 12: pp->setIsInvalidData(string2bool(value)); return true;
        case 13: pp->setTimeSlot(string2long(value)); return true;
        default: return false;
    }
}

const char *MyMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        "TransportAddress",
        "OverlayKey",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<14) ? fieldStructNames[field] : NULL;
}

void *MyMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    MyMessage *pp = (MyMessage *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getSenderAddress()); break;
        case 2: return (void *)(&pp->getKey()); break;
        default: return NULL;
    }
}


