EXPLAIN ANALYZE
SELECT
    Region,
    Product,
    Category,
    Store,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    CUBE (Region, Product, Category, Store, Quarter)
ORDER BY
    Region,
    Product,
    Category,
    Store,
    Quarter;