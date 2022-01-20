use crate::mangle::Mangle;
use serde::Deserialize;

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum InvalidDataType {
    SliceOfComplex,
}

impl std::fmt::Display for InvalidDataType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::SliceOfComplex => write!(f, "Slices of non-primitives are not supported"),
        }
    }
}

impl std::error::Error for InvalidDataType {}

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum Direction {
    In,
    Out,
}

#[derive(Debug, Clone, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum DataType {
    U8,
    U16,
    U32,
    U64,
    Usize,
    I8,
    I16,
    I32,
    I64,
    Bool,
    ConstPtr,
    MutPtr,
    Str,
    Slice(Box<DataType>),
    Wrapped(TypeName),
}

impl DataType {
    pub fn check(&self) -> Result<(), InvalidDataType> {
        match self {
            Self::Slice(inner) if !inner.is_primitive() => Err(InvalidDataType::SliceOfComplex),
            _ => Ok(()),
        }
    }

    pub fn is_primitive(&self) -> bool {
        match self {
            Self::Slice(_) | Self::Str => false,
            _ => true,
        }
    }

    pub fn c_types(&self) -> Vec<String> {
        match self {
            Self::U8 => vec![String::from("uint8_t")],
            Self::U16 => vec![String::from("uint16_t")],
            Self::U32 => vec![String::from("uint32_t")],
            Self::U64 => vec![String::from("uint64_t")],
            Self::Usize => vec![String::from("size_t")],
            Self::I8 => vec![String::from("int8_t")],
            Self::I16 => vec![String::from("int16_t")],
            Self::I32 => vec![String::from("int32_t")],
            Self::I64 => vec![String::from("int64_t")],
            Self::Bool => vec![String::from("bool")],
            Self::ConstPtr => vec![String::from("const void*")],
            Self::MutPtr => vec![String::from("void*")],
            Self::Str => vec![String::from("char*"), String::from("size_t")],
            Self::Slice(_) => vec![String::from("void*"), String::from("size_t")],
            Self::Wrapped(type_name) => vec![type_name.to_string()],
        }
    }

    pub fn cxx_type(&self, direction: Direction) -> String {
        match self {
            Self::U8 => String::from("uint8_t"),
            Self::U16 => String::from("uint16_t"),
            Self::U32 => String::from("uint32_t"),
            Self::U64 => String::from("uint64_t"),
            Self::Usize => String::from("size_t"),
            Self::I8 => String::from("int8_t"),
            Self::I16 => String::from("int16_t"),
            Self::I32 => String::from("int32_t"),
            Self::I64 => String::from("int64_t"),
            Self::Bool => String::from("bool"),
            Self::ConstPtr => String::from("const void*"),
            Self::MutPtr => String::from("void*"),
            Self::Str => String::from(match direction {
                Direction::In => "std::string_view",
                Direction::Out => "std::string",
            }),
            Self::Slice(data_type) => format!(
                "{}<{}>",
                match direction {
                    Direction::In => "std::span",
                    Direction::Out => "std::vector",
                },
                data_type.cxx_type(direction)
            ),
            Self::Wrapped(type_name) => type_name.to_string(),
        }
    }

    pub fn view_c_types<'a>(
        &'a self,
        sep: &'a mut impl FnMut() -> &'static str,
        base_name: &'a str,
    ) -> Result<String, std::fmt::Error> {
        let mut buf = String::new();
        self.write_c_types(&mut buf, sep, base_name)?;
        Ok(buf)
    }

    pub fn write_c_types(
        &self,
        out: &mut impl std::fmt::Write,
        sep: &mut impl FnMut() -> &'static str,
        base_name: &str,
    ) -> std::fmt::Result {
        let c_types = self.c_types();
        let multiple = c_types.len() > 1;
        for (i, c_type) in c_types.into_iter().enumerate() {
            if multiple {
                write!(
                    out,
                    "{sep}{t} {arg}_N{i}",
                    t = c_type,
                    arg = base_name,
                    i = i,
                    sep = sep(),
                )?;
            } else {
                write!(
                    out,
                    "{sep}{t} {arg}",
                    t = c_type,
                    arg = base_name,
                    sep = sep(),
                )?;
            }
        }
        Ok(())
    }
}

#[derive(Debug, Clone, Eq, PartialEq, Hash, Deserialize)]
pub struct TypeName(String);

impl TypeName {
    pub fn base(&self) -> &str {
        self.0.rsplit_once("::").map(|(_, x)| x).unwrap()
    }

    pub fn namespace(&self) -> &str {
        self.0.rsplit_once("::").map(|(x, _)| x).unwrap()
    }
}

impl Mangle for TypeName {
    fn mangle_to(&self, out: &mut impl std::fmt::Write) -> std::fmt::Result {
        self.0.as_str().mangle_to(out)
    }
}

impl std::fmt::Display for TypeName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}
