import System.Environment
import Data.List
import Debug.Trace

type VarType = Int
data Expr = Var VarType | Abs Int Expr | App Expr Expr deriving (Eq)

prettyprint x
        | x == 0 = "m"
        | x == 1 = "n"
        | x == 2 = "f"
        | x == 3 = "x"
        | x == 4 = "g"
        | x == 5 = "h"
        | x == 6 = "u"
        | otherwise = "x_"++show x

instance Show Expr where
    show (Abs x body)      = "(λ"++prettyprint x++" "++show body++") "
    show (App exprl exprr) = "["++show exprl++" "++show exprr++"] "
    show (Var x)           = prettyprint x

(without) (x:xs) e
    | x == e    = without xs e
    | otherwise = x:(without xs e)
(without) [] e  = []

freeVariables :: Expr -> [Int]
freeVariables (Var x)           = [x]
freeVariables (Abs x body)      = (freeVariables body) `without` x
freeVariables (App exprl exprr) = nub $ (freeVariables exprl)++(freeVariables exprr)

rename :: Expr -> Int -> Int -> Expr
rename (Var x) y z
    | x == y                 = Var z
    | otherwise              = Var x
rename (Abs x body) y z
    | x == y                 = (Abs z (rename body y z))
    | otherwise              = (Abs x (rename body y z))
rename (App exprl exprr) y z = App (rename exprl y z) (rename exprr y z)

substituion :: Expr -> Expr -> Expr -> Expr
substituion (Var x) (Var y) z
    | x == y                                     = z
    | otherwise                                  = Var x
substituion (Abs x body) (Var y) z
    | x == y                                     = Abs x body
    | x /= y && (not $ elem x (freeVariables z)) = Abs x (substituion body (Var y) z)
    | otherwise                                  = Abs nextFree $ substituion (rename body x nextFree) (Var y) z
    where
        nextFree = if length zFVs == 0 then 0 else maximum zFVs + 1
        zFVs     = freeVariables z
substituion (App exprl exprr) y z                = App (substituion exprl y z) (substituion exprr y z)

betaReduction :: Expr -> Expr
betaReduction expr@(App (Abs x body) argument) = substituion body (Var x) argument
-- betaReduction expr@(App (Abs x body) argument) = trace (prettyprint x++" --> "++show argument) $ substituion body (Var x) argument

-- this function will not terminate when applied to an expression that
-- has no normal form
reduceToNormalForm expr
    | alphaEquivalence expr result = result
    -- | otherwise                    = trace (show expr++"\n") $ reduceToNormalForm result
    | otherwise                    = reduceToNormalForm result
    where
        result = _reduceToNormalForm expr
        _reduceToNormalForm expr@(App (Abs x body) argument) = betaReduction $ App (Abs x body) argument
        _reduceToNormalForm expr@(Var x)                     = expr
        _reduceToNormalForm expr@(Abs x body)                = Abs x (_reduceToNormalForm body)
        _reduceToNormalForm expr@(App exprl exprr)           = App (_reduceToNormalForm exprl) (_reduceToNormalForm exprr)

alphaConversion expr var = _alphaConversion expr var []
    where
        _alphaConversion expr var bound
            | exprFVs == [] = result
            | var <= (maximum $ exprFVs) = Nothing
            | otherwise = result
            where
                exprFVs = freeVariables expr
                result =  case expr of (Var x)            -> if x `elem` bound then (Just $ Var x) else (Just $ Var var)
                                       (Abs x body)       -> do
                                                                convertedBody <- (_alphaConversion (rename body x var) (var+1) (var:bound));
                                                                return (Abs var convertedBody);
                                       (App exprl exprr)  -> do
                                                                convertedLeft <- (_alphaConversion exprl var bound);
                                                                convertedRight <- (_alphaConversion exprr var bound);
                                                                return (App convertedLeft convertedRight);

alphaEquivalence :: Expr -> Expr -> Bool
alphaEquivalence expr1 expr2 = alphaConvExpr1 == alphaConvExpr2
    where
        firstCommonFreeVar = if length expr1_expr2_FVs == 0 then 0 else (maximum $ expr1_expr2_FVs) + 1
        alphaConvExpr1     = alphaConversion expr1 firstCommonFreeVar
        alphaConvExpr2     = alphaConversion expr2 firstCommonFreeVar
        expr1_expr2_FVs    = (freeVariables expr1)++(freeVariables expr2)

add e1 e2 = reduceToNormalForm $ App (App _add e1) e2
    where
        _add = (Abs m (Abs n (Abs f (Abs x (App (App (Var m) (Var f)) (App (App (Var n) (Var f) ) (Var x)))))))
        m = 0
        n = 1
        f = 2
        x = 3

subtract e1 e2 = reduceToNormalForm $ App (App _sub e2) e1
    where
        _prd = (Abs n (Abs f (Abs x (App (App (App (Var n) (Abs g (Abs h (App (Var h) (App (Var g) (Var f)))))) (Abs u (Var x))) (Abs u (Var u))))))
        _sub = (Abs e2 (Abs e1 (App (App e2 _prd) e1)))
        m = 0
        n = 1
        f = 2
        x = 3

suc :: Expr -> Expr
suc e = reduceToNormalForm $ App _suc e
    where
        _suc = (Abs n (Abs f (Abs x (App (Var f) (App (App (Var n) (Var f)) (Var x))))))
        n = 1
        f = 2
        x = 3

prd :: Expr -> Expr
prd e = reduceToNormalForm $ App _prd e
    where
        _prd = (Abs n (Abs f (Abs x (App (App (App (Var n) (Abs g (Abs h (App (Var h) (App (Var g) (Var f)))))) (Abs u (Var x))) (Abs u (Var u))))))
        n = 1
        f = 2
        x = 3
        g = 4
        h = 5
        u = 6


numeral :: Int -> Expr
numeral n = (Abs f (Abs x $ _numeral n))
    where
        _numeral n
           | n == 0 = (Var x)
           | otherwise = (App (Var f) (_numeral $ n-1))
        f = 2
        x = 3

arabic :: Expr -> Int
arabic (Abs f (Abs x expr)) = arabic expr
arabic (App exprl expr)     = 1 + (arabic expr)
arabic (Var x)              = 0

main = do
    -- let expr1 = Abs 2 (Var 1)
    -- print $ substituion expr1 (Var 1) (Var 2)

    -- let expr = (Abs 1 (App (Var 4) (Var 1)))
    -- let Just alphaConvExpr = alphaConversion expr 5
    -- print $ expr == alphaConvExpr
    -- print $ alphaEquivalence expr alphaConvExpr

    -- let zero = numeral 0
    -- print . arabic $ add zero zero

    args <- getArgs
    let n = (read $ head args :: Int)
    print $ arabic $ numeral n

    return ()





















