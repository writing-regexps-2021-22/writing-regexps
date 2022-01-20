use crate::datatype::{DataType, TypeName};
use serde::Deserialize;
use crate::mangle::Mangle;

#[derive(Debug, Clone, Deserialize)]
pub struct Argument {
    pub name: String,
    #[serde(rename = "type")]
    pub arg_type: DataType,
}

#[derive(Debug, Clone, Deserialize)]
pub struct Function {
    #[serde(rename = "self")]
    pub self_kind: SelfKind,
    pub args: Vec<Argument>,
    #[serde(rename = "return")]
    pub return_type: DataType,
}

#[derive(Debug, Copy, Clone, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum SelfKind {
    Ref,
    Mut
}

impl SelfKind {
    pub fn as_ptr(self) -> DataType {
        match self {
            Self::Ref => DataType::ConstPtr,
            Self::Mut => DataType::MutPtr,
        }
    }
}

#[derive(Debug, Clone)]
pub struct Method {
    pub type_name: TypeName,
    pub method_name: String,
}

impl Mangle for Method {
    fn mangle_to(&self, out: &mut impl std::fmt::Write) -> std::fmt::Result {
        write!(out, "_rustcxx_binding_")?;
        self.type_name.mangle_to(out)?;
        write!(out, "_m_")?;
        self.method_name.as_str().mangle_to(out)?;
        Ok(())
    }
}
