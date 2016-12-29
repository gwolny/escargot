#include "Escargot.h"
#include "GlobalObject.h"
#include "Context.h"
#include "DateObject.h"
#include "ErrorObject.h"

namespace Escargot {

#define FOR_EACH_DATE_VALUES(F) \
    F(Date)                     \
    F(FullYear)                 \
    F(Hours)                    \
    F(Minutes)                  \
    F(Month)                    \
    F(UTCDate)                  \
    F(UTCFullYear)              \
    F(UTCHours)                 \
    F(UTCMinutes)               \
    F(UTCMonth)                 \
    F(Milliseconds)             \
    F(Seconds)                  \
    F(UTCMilliseconds)          \
    F(UTCSeconds)


static Value builtinDateConstructor(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    DateObject* thisObject;
    if (isNewExpression) {
        thisObject = thisValue.asObject()->asDateObject();

        if (argc == 0) {
            thisObject->setTimeValueAsCurrentTime();
        } else if (argc == 1) {
            Value v = argv[0].toPrimitive(state);
            if (v.isString()) {
                thisObject->setTimeValue(state, v);
            } else {
                double V = v.toNumber(state);
                thisObject->setTimeValue(state, DateObject::timeClip(state, V));
            }
        } else {
            double args[7] = { 0, 0, 1, 0, 0, 0, 0 }; // default value of year, month, date, hour, minute, second, millisecond
            for (size_t i = 0; i < argc; i++) {
                args[i] = argv[i].toNumber(state);
            }
            double year = args[0];
            double month = args[1];
            double date = args[2];
            double hour = args[3];
            double minute = args[4];
            double second = args[5];
            double millisecond = args[6];
            if ((int)year >= 0 && (int)year <= 99) {
                year += 1900;
            }
            if (std::isnan(year) || std::isnan(month) || std::isnan(date) || std::isnan(hour) || std::isnan(minute) || std::isnan(second) || std::isnan(millisecond)) {
                thisObject->setTimeValueAsNaN();
                return new ASCIIString("Invalid Date");
            }
            thisObject->setTimeValue(state, (int)year, (int)month, (int)date, (int)hour, (int)minute, second, millisecond);
        }
        return thisObject->toFullString(state);
    } else {
        DateObject d(state);
        d.setTimeValueAsCurrentTime();
        return d.toFullString(state);
    }
}

static Value builtinDateNow(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateParse(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateUTC(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateGetTime(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    RESOLVE_THIS_BINDING_TO_OBJECT(thisObject, Date, getTime);
    if (!thisObject->isDateObject()) {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Date.string(), true, state.context()->staticStrings().getTime.string(), errorMessage_GlobalObject_ThisNotDateObject);
    }
    return Value(thisObject->asDateObject()->primitiveValue());
}

static Value builtinDateValueOf(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    RESOLVE_THIS_BINDING_TO_OBJECT(thisObject, Date, valueOf);
    if (!thisObject->isDateObject()) {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Date.string(), true, state.context()->staticStrings().valueOf.string(), errorMessage_GlobalObject_ThisNotDateObject);
    }
    double val = thisObject->asDateObject()->primitiveValue();
    return Value(val);
}

static Value builtinDateToString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    RESOLVE_THIS_BINDING_TO_OBJECT(thisObject, Date, toString);
    if (!thisObject->isDateObject()) {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Date.string(), true, state.context()->staticStrings().toString.string(), errorMessage_GlobalObject_ThisNotDateObject);
    }
    return thisObject->asDateObject()->toFullString(state);
}

static Value builtinDateToDateString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateToTimeString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateToLocaleString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateToLocaleDateString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateToLocaleTimeString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateToUTCString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateToISOString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinDateToJSON(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    state.throwException(new ASCIIString(errorMessage_NotImplemented));
    RELEASE_ASSERT_NOT_REACHED();
}

typedef int (*DateGetterInt)(ExecutionState& state);
typedef int64_t (*DateGetterInt64)(ExecutionState& state);

#define DECLARE_STATIC_DATE_GETTER(Name)                                                                                                                                                                                           \
    static Value builtinDateGet##Name(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)                                                                                                      \
    {                                                                                                                                                                                                                              \
        RESOLVE_THIS_BINDING_TO_OBJECT(thisObject, Date, get##Name);                                                                                                                                                               \
        if (!thisObject->isDateObject()) {                                                                                                                                                                                         \
            ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Date.string(), true, state.context()->staticStrings().get##Name.string(), errorMessage_GlobalObject_ThisNotDateObject); \
        }                                                                                                                                                                                                                          \
        if (!thisObject->asDateObject()->isValid())                                                                                                                                                                                \
            return Value(std::numeric_limits<double>::quiet_NaN());                                                                                                                                                                \
        return Value(thisObject->asDateObject()->get##Name(state));                                                                                                                                                                \
    }

FOR_EACH_DATE_VALUES(DECLARE_STATIC_DATE_GETTER);
DECLARE_STATIC_DATE_GETTER(Day);
DECLARE_STATIC_DATE_GETTER(UTCDay);
DECLARE_STATIC_DATE_GETTER(TimezoneOffset);

#define DECLARE_STATIC_DATE_SETTER(Name)                                                                                      \
    static Value builtinDateSet##Name(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression) \
    {                                                                                                                         \
        state.throwException(new ASCIIString(errorMessage_NotImplemented));                                                   \
        RELEASE_ASSERT_NOT_REACHED();                                                                                         \
    }

FOR_EACH_DATE_VALUES(DECLARE_STATIC_DATE_SETTER);

static Value builtinDateSetTime(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    RESOLVE_THIS_BINDING_TO_OBJECT(thisObject, Date, setTime);
    if (!thisObject->isDateObject()) {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Date.string(), true, state.context()->staticStrings().setTime.string(), errorMessage_GlobalObject_ThisNotDateObject);
    }
    if (argc > 0 && argv[0].isNumber()) {
        thisObject->asDateObject()->setTime(argv[0].toNumber(state));
        return Value(thisObject->asDateObject()->primitiveValue());
    } else {
        double value = std::numeric_limits<double>::quiet_NaN();
        thisObject->asDateObject()->setTimeValueAsNaN();
        return Value(value);
    }
}


void GlobalObject::installDate(ExecutionState& state)
{
    m_date = new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().Date, builtinDateConstructor, 7, [](ExecutionState& state, size_t argc, Value* argv) -> Object* {
                                    return new DateObject(state);
                                }),
                                FunctionObject::__ForBuiltin__);
    m_date->markThisObjectDontNeedStructureTransitionTable(state);
    m_date->setPrototype(state, m_functionPrototype);
    m_datePrototype = m_objectPrototype;
    m_datePrototype = new Object(state);
    m_datePrototype->setPrototype(state, m_objectPrototype);

    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().constructor), ObjectPropertyDescriptor(m_date, (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_date->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().now),
                              ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().now, builtinDateNow, 0, nullptr, NativeFunctionInfo::Strict)),
                                                       (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_date->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().parse),
                              ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().parse, builtinDateParse, 0, nullptr, NativeFunctionInfo::Strict)),
                                                       (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_date->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().UTC),
                              ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().UTC, builtinDateUTC, 0, nullptr, NativeFunctionInfo::Strict)),
                                                       (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().getTime),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().getTime, builtinDateGetTime, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().setTime),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().setTime, builtinDateSetTime, 1, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().valueOf),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().valueOf, builtinDateValueOf, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toString),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toString, builtinDateToString, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toDateString),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toDateString, builtinDateToDateString, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toTimeString),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toTimeString, builtinDateToTimeString, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toLocaleString),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toLocaleString, builtinDateToLocaleString, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toLocaleDateString),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toLocaleDateString, builtinDateToLocaleDateString, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toLocaleTimeString),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toLocaleTimeString, builtinDateToLocaleTimeString, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toUTCString),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toUTCString, builtinDateToUTCString, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toISOString),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toISOString, builtinDateToISOString, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toJSON),
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toJSON, builtinDateToJSON, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

#define DATE_DEFINE_GETTER(dname)                                                                                                                                                                                          \
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().get##dname),                                                                                                             \
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().get##dname, builtinDateGet##dname, 0, nullptr, NativeFunctionInfo::Strict)), \
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    FOR_EACH_DATE_VALUES(DATE_DEFINE_GETTER);
    DATE_DEFINE_GETTER(Day);
    DATE_DEFINE_GETTER(UTCDay);
    DATE_DEFINE_GETTER(TimezoneOffset);

#define DATE_DEFINE_SETTER(dname)                                                                                                                                                                                          \
    m_datePrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().set##dname),                                                                                                             \
                                       ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().set##dname, builtinDateSet##dname, 1, nullptr, NativeFunctionInfo::Strict)), \
                                                                (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    FOR_EACH_DATE_VALUES(DATE_DEFINE_SETTER);

    m_date->setFunctionPrototype(state, m_datePrototype);

    defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().Date),
                      ObjectPropertyDescriptor(m_date, (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
}
}
